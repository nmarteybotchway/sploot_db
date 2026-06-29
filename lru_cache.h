#ifndef LRU_CACHE_H
#define LRU_CACHE_H
#include <optional>
#include <list>
#include <unordered_map>
#include <string>
#include <iterator>
class LRU
{
public:
    LRU(int capacity);
    bool get(const std::string &key);
    void put(const std::string &key);
    void remove(const std::string &key);
private:
    int capacity;
    std::list<std::string> list;
    std::unordered_map<std::string, std::list<std::string>::iterator> cache_map;

};

#endif // LRU_CACHE_H
