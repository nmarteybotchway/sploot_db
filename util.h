#ifndef UTIL_H
#define UTIL_H
#include <unordered_map>
#include <string>
#include <map>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sys/stat.h>
#include <regex>
#include <optional>

class Util
{
public:
    static inline bool file_exists(const std::string& name) {
        struct stat buffer;
        return (stat (name.c_str(), &buffer) == 0);
    }

    static int extract_sst_num(const std::string& filename);

    static std::optional<std::string> find_key_from_sstables(const std::vector<std::string> &ss_tables, const std::string &key);

    static void flush(int &counter, std::map<std::string, std::string> &memtable, std::vector<std::string> &ss_tables);

    static void initialise_manifest(int &counter, std::vector<std::string> &ss_tables);
};

#endif // UTIL_H
