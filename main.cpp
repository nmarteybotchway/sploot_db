#include <iostream>
#include "httplib.h"
#include <unordered_map>
#include <string>


int main()
{
    using namespace httplib;
    std::unordered_map<std::string, std::string> storage;
    Server svr;
    svr.Get("/:key", [&storage](const Request& req, Response& res) {
        std::string key = req.path_params.at("key");
        if (storage.contains(key)) {
            res.set_content(storage.at(key), "text/plain");
            res.status = 200;
        } else {
            res.set_content("Key not found", "text/plain");
            res.status = 404;
        }
    });

    svr.Put("/:key", [&storage](const Request& req, Response& res) {
        std::string key = req.path_params.at("key");
        std::cout << key << std::endl;
        if (storage.contains(key)) {
            storage[key] = req.body;
        } else {
            storage.insert({key, req.body});
        }
        res.status = 200;
    });

    svr.listen("0.0.0.0", 8000);
    return 0;
}
