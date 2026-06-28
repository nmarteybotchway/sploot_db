#include <iostream>
#include "httplib.h"
#include <unordered_map>
#include <string>
#include <map>
#include <optional>
#include "util.h"

int main()
{
    using namespace httplib;

    const int MAX_MEMTABLE_SIZE = 2;

    std::unordered_map<std::string, std::string> storage;
    std::map<std::string, std::string> memtable;
    std::vector<std::string> ss_tables{};
    int counter{};

    Util::initialise_manifest(counter, ss_tables);

    Server svr;

    svr.Get("/:key", [&memtable, &ss_tables](const Request& req, Response& res) {
        std::string key = req.path_params.at("key");
        if (memtable.contains(key)) {
            res.set_content(memtable.at(key), "text/plain");
            res.status = 200;
        } else {
            auto result = Util::find_key_from_sstables(ss_tables, key);
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
        memtable[key] = req.body;
        if (memtable.size() == MAX_MEMTABLE_SIZE) {
            Util::flush(counter, memtable, ss_tables);
        }

        res.status = 200;
    });

    svr.listen("0.0.0.0", 8000);
    return 0;
}
