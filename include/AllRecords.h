#ifndef AllRecords_h
#define AllRecords_h

#include "LockedChessAllInterface.h"
#include <nlohmann/json.hpp>
#include "CacheCleaner.h"


class AllRecords {  
private:
    static std::recursive_mutex recordsLock;
    static nlohmann::json records;
    static std::map<std::string, std::string> getRecordCache;
    static std::map<std::string, int> getRecordCacheCount;
    static std::map<std::string, std::chrono::system_clock::time_point> getRecordCacheTime;
    static std::shared_mutex getRecordCacheLock;
    static std::shared_mutex getRecordCacheCountLock;
    friend class OtherRecords;

public:
    AllRecords() = delete;
    static void  addRecord(std::shared_ptr<WriterLockedChessAllInterface> game, const std::string& name="");
    static void  writeRecords(std::string filename);
    static void  readRecords(std::string filename);
    static void  deleteRecord(std::string name);
    static std::string  getRecord();
    static std::string  readRecord(std::string boardRecord, std::string operationRecord, std::string timeRecord);
    static void  cleanAllRecordsCache();
};

class OtherRecords {
private:
    static std::recursive_mutex otherRecordsLock;
    static nlohmann::json otherRecords;
    static bool hasCombined;
public:
    OtherRecords() = delete;
    static void  readRecords(std::string filename);
    static void  combineRecords();
};

#endif // AllRecords_h
