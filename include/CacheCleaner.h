#ifndef CACHECLEANER_H
#define CACHECLEANER_H

#include <map>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <chrono>


namespace CacheCleaner {
    template<typename T>
    void  cleanCache(std::map<std::string, T>& cache,
                   std::map<std::string, int>& countCache,
                   std::map<std::string, std::chrono::system_clock::time_point>& timeCache,
                   std::shared_mutex& cacheLock,
                   std::shared_mutex& cacheCountLock);
                   
    template<typename T>
    void cleanCache(std::map<std::string, T>& cache,
                   std::map<std::string, int>& countCache,
                   std::map<std::string, std::chrono::system_clock::time_point>& timeCache,
                   std::shared_mutex& cacheLock,
                   std::shared_mutex& cacheCountLock) {
        std::unique_lock<std::shared_mutex> locker(cacheLock);
        std::unique_lock<std::shared_mutex> countLocker(cacheCountLock);
        
        auto currentTime = std::chrono::system_clock::now();
        for (auto it = timeCache.begin(); it != timeCache.end(); ) {
            if (it->second + std::chrono::seconds(60) < currentTime && 
                (countCache[it->first] < 10)) {
                cache.erase(it->first);
                countCache.erase(it->first);
                it = timeCache.erase(it);
            } else {
                timeCache[it->first] = currentTime;
                countCache[it->first]--;
                if (countCache[it->first] < 1) {
                    countCache[it->first] = 1;
                }
                ++it;
            }
        }
    }
}

#endif // CACHECLEANER_H