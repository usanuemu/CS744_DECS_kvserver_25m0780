#include "httplib.h"
#include <iostream>
#include <string>
#include <list>
#include <unordered_map>
#include <mutex>


const std::string BACKEND_HOST = "localhost";
const int BACKEND_PORT = 5003;
const int CACHE_MAX_SIZE = 3;


class LRUCache {
    private:

    unsigned int cacheSize;
    std::list<std::string> doubleLinkedList; // list that stores lru keys at the end
    std::unordered_map<std::string, std::string> cacheMap;
    std::mutex cacheMutex;

    public:
    LRUCache(unsigned int c){
        cacheSize = c;
    }

    bool get(const std::string& key, std::string& value_out) {
            std::lock_guard<std::mutex> lock(cacheMutex);

            auto it = cacheMap.find(key);
            if (it == cacheMap.end()) {
                return false;
            }

            doubleLinkedList.remove(key);
            doubleLinkedList.push_front(key);

            value_out = it->second;
            return true;
        }

        void put(const std::string& key, const std::string& value) {
            std::lock_guard<std::mutex> lock(cacheMutex);

            auto it = cacheMap.find(key);
            if (it != cacheMap.end()) {
                doubleLinkedList.remove(key);
            }
            else if (cacheMap.size() == cacheSize) {
                std::string lru_key = doubleLinkedList.back();
                doubleLinkedList.pop_back();
                cacheMap.erase(lru_key);
                std::cout << "Removed LRU key: " << lru_key << std::endl;
            }
            doubleLinkedList.push_front(key);
            cacheMap[key] = value;
        }

        void evict(const std::string& key) {
            std::lock_guard<std::mutex> lock(cacheMutex);

            auto it = cacheMap.find(key);
            if (it != cacheMap.end()) {
                doubleLinkedList.remove(key);
                cacheMap.erase(it);
            }
        }
};

LRUCache cache(CACHE_MAX_SIZE);

int main() {
    using namespace httplib;
    Server svr;

    svr.Get("/kv/:key", [&](const Request& req, Response& res) {
        std::string key = req.path_params.at("key");

        std::string cached_value;
        if (cache.get(key, cached_value)){
            res.set_content(cached_value, "text/plain");
            std::cout << "[Frontend] GET: Key '" << key << "' (from CACHE)" << std::endl;
            return;
        }

        std::cout << "[Frontend] GET: Key '" << key << "' (CACHE MISS)" << std::endl;
        Client cli(BACKEND_HOST, BACKEND_PORT);
        auto backend_res = cli.Get(req.path.c_str());

        if (backend_res) {
            res.status = backend_res->status;
            res.set_content(backend_res->body, "text/plain");

            if (backend_res->status == 200) {
                cache.put(key, backend_res->body);
                std::cout << "[Frontend] Cache updated for key '" << key << "'" << std::endl;
            }
        }
    });

    svr.Post("/kv/:key", [&](const Request& req, Response& res) {
        std::string key = req.path_params.at("key");

        Client cli(BACKEND_HOST, BACKEND_PORT);
        auto backend_res = cli.Post(req.path.c_str(), req.body, "text/plain");

        if (backend_res) {
            res.status = backend_res->status;
            res.set_content(backend_res->body, "text/plain");

            if (backend_res->status == 200) {
                cache.put(key, req.body);
                std::cout << "[Frontend] POST: Key '" << key << "'. Cache updated." << std::endl;
            }
        }
    });

    svr.Delete("/kv/:key", [&](const Request& req, Response& res) {
        std::string key = req.path_params.at("key");

        Client cli(BACKEND_HOST, BACKEND_PORT);
        auto backend_res = cli.Delete(req.path.c_str());

        if (backend_res) {
            res.status = backend_res->status;
            res.set_content(backend_res->body, "text/plain");

            if (backend_res->status == 200) {
                cache.evict(key);
                std::cout << "[Frontend] DELETE: Key '" << key << "'. Key removed from cache." << std::endl;
            }
        }
    });

    std::cout << "Frontend server listening on http://0.0.0.0:5002\n"<< std::endl;
    svr.listen("0.0.0.0", 5002);

    return 0;
}
