#include <iostream>
#include "httplib.h"
#include <unordered_map>
#include <string>
#include <map>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sys/stat.h>
#include <regex>
#include <optional>

using json = nlohmann::json;

const int MAX_MEMTABLE_SIZE = 2;

inline bool file_exists(const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

int extract_sst_num(const std::string& filename) {
    std::regex sst_pattern("sst-(\\d+)\\.json");
    std::smatch match;

    if (std::regex_search(filename, match, sst_pattern)) {
        std::string num_str = match[1].str();
        return std::stoi(num_str);
    }
    return -1;
}

std::optional<std::string> find_key_from_sstables(const std::vector<std::string> &ss_tables, const std::string &key) {
    for (const auto &filename : ss_tables) {
        std::ifstream f(filename);
        if (!f.is_open()) {
            continue;
        }

        if (f.peek() == std::ifstream::traits_type::eof()) {
            continue;
        }
        json data;
        try {
            data = json::parse(f);
        } catch (const json::parse_error&) {
            continue;
        }
        if (!data.is_array()) {
            continue;
        }

        for (const auto &element : data) {
            if (element.is_object() && element.contains("key")) {
                if (element["key"] == key) {
                    return element.value("value", "");
                }
            }
        }
    }

    return std::nullopt;
}

void flush(int &counter, std::map<std::string, std::string> &memtable, std::vector<std::string> &ss_tables) {
    // flush memtable and write to sstable
    std::string filename = std::format("sst-{}.json", counter);
    json j_array = json::array();

    for (const auto& [key, value] : memtable) {
        json item;
        item["key"] = key;
        item["value"] = value;
        j_array.push_back(item);
    }

    std::ofstream o1(filename);
    if (o1.is_open()) {
        o1 << std::setw(4) << j_array << std::endl;
    }

    o1.close();

    ss_tables.insert(ss_tables.begin(), filename);
    std::ofstream o("MANIFEST", std::ios::trunc);
    if (o.is_open()) {
        json data = ss_tables;
        o << data.dump(4);
        o.close();
    }
    // flushing step
    memtable.clear();
    // increment counter
    counter++;
}


int main()
{
    int counter{};
    std::vector<std::string> ss_tables{};
    if (!file_exists("MANIFEST")) {
        std::ofstream o("MANIFEST");
        o.close();
    } else {
        // first read the file, deserialize from json to std::vector
        std::ifstream f("MANIFEST");

        json data = json::parse(f);
        data.get_to(ss_tables);
        // first element of vector was most recently added filename,
        // use extract_sst_num to extract the latest sst_num, used
        // to initialise the counter from where we last left off.
        counter = extract_sst_num(ss_tables[0]) + 1;

    }
    using namespace httplib;

    std::unordered_map<std::string, std::string> storage;
    std::map<std::string, std::string> memtable;

    Server svr;
    svr.Get("/:key", [&memtable, &ss_tables](const Request& req, Response& res) {
        std::string key = req.path_params.at("key");
        if (memtable.contains(key)) {
            res.set_content(memtable.at(key), "text/plain");
            res.status = 200;
        } else {
            auto result = find_key_from_sstables(ss_tables, key);
            if (result.has_value()) {
                res.set_content(result.value(), "text/plain");
                res.status = 200;
            }
            else {
                res.set_content("Key not found", "text/plain");
                res.status = 404;
            }

        }
    });

    svr.Put("/:key", [&storage, &memtable, &counter, &ss_tables](const Request& req, Response& res) {
        std::string key = req.path_params.at("key");
        if (storage.contains(key)) {
            storage[key] = req.body;
        } else {
            storage.insert({key, req.body});
        }
        std::cout << memtable.size() << std::endl;

        memtable[key] = req.body;
        if (memtable.size() == MAX_MEMTABLE_SIZE) {
            flush(counter, memtable, ss_tables);
        }

        res.status = 200;
    });

    svr.listen("0.0.0.0", 8000);
    return 0;
}
