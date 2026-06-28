#include "util.h"

int Util::extract_sst_num(const std::string& filename) {
    std::regex sst_pattern("sst-(\\d+)\\.json");
    std::smatch match;

    if (std::regex_search(filename, match, sst_pattern)) {
        std::string num_str = match[1].str();
        return std::stoi(num_str);
    }
    return -1;
}

std::optional<std::string> Util::find_key_from_sstables(const std::vector<std::string> &ss_tables, const std::string &key) {
    using json = nlohmann::json;

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

void Util::flush(int &counter, std::map<std::string, std::string> &memtable, std::vector<std::string> &ss_tables) {
    using json = nlohmann::json;
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

void Util::initialise_manifest(int &counter, std::vector<std::string> &ss_tables) {
    using json = nlohmann::json;

    if (!file_exists("MANIFEST")) {
        std::ofstream o("MANIFEST");
        o.close();
    } else {
        std::ifstream f("MANIFEST");
        json data = json::parse(f);
        data.get_to(ss_tables);
        counter = Util::extract_sst_num(ss_tables[0]) + 1;
    }
}
