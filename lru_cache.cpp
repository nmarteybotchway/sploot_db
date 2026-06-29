#include "lru_cache.h"

LRU::LRU(int capacity) {
    this->capacity = capacity;
}

bool LRU::get(const std::string &key) {
    auto it = cache_map.find(key);
    if (it == cache_map.end()) {
        return false;
    }
    list.erase(it->second);
    list.push_front(key);

    cache_map[key] = list.begin();
    return true;
}

void LRU::put(const std::string &key) {
    auto it = cache_map.find(key);
    if (it != cache_map.end()) {
        list.erase(it->second);
        cache_map.erase(it);
    }
    list.push_front(key);
    cache_map[key] = list.begin();

    if (cache_map.size() > capacity) {
        auto last_node = list.back();
        list.pop_back();
        cache_map.erase(last_node);
    }
}

void LRU::remove(const std::string &key) {
    auto it = cache_map.find(key);
    if (it != cache_map.end()) {
        list.erase(it->second);
        cache_map.erase(it);
    }
}