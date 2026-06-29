#include <iostream>
#include "httplib.h"
#include <unordered_map>
#include <string>
#include <map>
#include <optional>
#include "util.h"
#include "lru_cache.h"

int main()
{
    using namespace httplib;

    const int MAX_MEMTABLE_SIZE = 2;
    const int NEGATIVE_CACHE_SIZE = 10;
    std::unordered_map<std::string, std::string> storage;
    std::map<std::string, std::string> memtable;
    std::vector<std::string> ss_tables{};
    LRU negative_cache{NEGATIVE_CACHE_SIZE};
    int counter{};

    Util::initialise_manifest(counter, ss_tables);

    Server svr;

    svr.Get("/:key", [&memtable, &ss_tables, &negative_cache](const Request& req, Response& res) {
        std::string key = req.path_params.at("key");
        if (memtable.contains(key)) {
            res.set_content(memtable.at(key), "text/plain");
            res.status = 200;
        } else {
            if (negative_cache.get(key)) {
                res.set_content("Key not found", "text/plain");
                res.status = 404;
            } else {
                auto result = Util::find_key_from_sstables(ss_tables, key);
                if (result.has_value()) {
                    res.set_content(result.value(), "text/plain");
                    res.status = 200;
                }
                else {
                    negative_cache.put(key);
                    res.set_content("Key not found", "text/plain");
                    res.status = 404;
                }
            }
        }
    });

    svr.Put("/:key", [&storage, &memtable, &counter, &ss_tables, &negative_cache](const Request& req, Response& res) {
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

        if (negative_cache.get(key)) {
            negative_cache.remove(key);
        }

        res.status = 200;
    });

    svr.listen("0.0.0.0", 8000);
    return 0;
}
