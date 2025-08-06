#include "AllRecords.h"
#include <fstream>
#include <sstream>
#include <chrono>

std::recursive_mutex AllRecords::recordsLock;
nlohmann::json AllRecords::records;
std::map<std::string, std::string> AllRecords::getRecordCache;
std::map<std::string, int> AllRecords::getRecordCacheCount;
std::map<std::string, std::chrono::system_clock::time_point> AllRecords::getRecordCacheTime;
std::shared_mutex AllRecords::getRecordCacheLock;
std::shared_mutex AllRecords::getRecordCacheCountLock;

std::recursive_mutex OtherRecords::otherRecordsLock;
nlohmann::json OtherRecords::otherRecords;
bool OtherRecords::hasCombined = false;

void AllRecords::addRecord(std::shared_ptr<WriterLockedChessAllInterface> game, const std::string& name) {
    std::unique_lock lock(recordsLock);
    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    std::string timeString = ss.str();
    nlohmann::json json = nlohmann::json::parse(game->getFinalResult());
    json["name"] = name;
    records[timeString] = json;
}

void AllRecords::writeRecords(std::string filename) {
    std::unique_lock lock(recordsLock);
    std::ofstream file(filename);
    file << records.dump(4);
    file.close();
}

void AllRecords::readRecords(std::string filename) {
    std::unique_lock lock(recordsLock);
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        records = nlohmann::json::parse(contents);
        file.close();
    }
}

void AllRecords::deleteRecord(std::string name) {
    std::unique_lock lock(recordsLock);
    records.erase(name);
}

std::string AllRecords::getRecord() {return records.dump(4);}
std::string AllRecords::readRecord(std::string boardRecord, std::string operationRecord, std::string timeRecord) {
    {
        std::shared_lock lock(getRecordCacheLock);
        if (getRecordCache.find(boardRecord + "&" + operationRecord + "&" +  timeRecord) != getRecordCache.end()) {
            std::unique_lock countlock(getRecordCacheCountLock);
            getRecordCacheCount[boardRecord + "&" + operationRecord + "&" +  timeRecord]++;
            getRecordCacheTime[boardRecord + "&" + operationRecord + "&" +  timeRecord] = std::chrono::system_clock::now();
            return getRecordCache[boardRecord + "&" + operationRecord + "&" +  timeRecord];
        }
    }
    auto boardIt = splitString(boardRecord, '#');
    auto operationIt = splitString(operationRecord, '#');
    auto timeIt = splitString(timeRecord, '#');
    auto startEndTime = splitString(timeIt.back(), '$');
    int n = boardIt.size();
    if (boardIt.size() != operationIt.size() || boardIt.size() != timeIt.size()) {
        throw std::runtime_error("Invalid record");
    }
    nlohmann::json finalResult;
    nlohmann::json oneStep;
    oneStep["board"] = boardIt[0];
    oneStep["operation"] = "无";
    oneStep["time"] = "0.00";
    finalResult["0"] = oneStep;
    finalResult["start_time"] = startEndTime.at(0);
    finalResult["end_time"] = startEndTime.at(1);
    finalResult["result"] = operationIt.back();
    for (int i = 1; i < n; ++i) {
        oneStep["board"] = boardIt[i];
        oneStep["operation"] = operationIt[i - 1];
        oneStep["time"] = timeIt[i - 1];
        finalResult[std::to_string(i)] = oneStep;
    }
    finalResult["step_number"] = n - 1;
    {
        std::unique_lock lock(getRecordCacheLock);
        getRecordCache[boardRecord + "&" + operationRecord + "&" +  timeRecord] = finalResult.dump(4);
        std::unique_lock countlock(getRecordCacheCountLock);
        getRecordCacheCount[boardRecord + "&" + operationRecord + "&" +  timeRecord] = 1;
        getRecordCacheTime[boardRecord + "&" + operationRecord + "&" +  timeRecord] = std::chrono::system_clock::now();
    }
    return finalResult.dump(4);
}

void AllRecords::cleanAllRecordsCache() {
    CacheCleaner::cleanCache(getRecordCache, getRecordCacheCount, getRecordCacheTime, getRecordCacheLock, getRecordCacheCountLock);
}

void OtherRecords::readRecords(std::string filename) {
    std::unique_lock lock(otherRecordsLock);
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        otherRecords = nlohmann::json::parse(contents);
        file.close();
        hasCombined = false;
    }
}

void OtherRecords::combineRecords() {
    std::unique_lock otherlock(otherRecordsLock);
    if (hasCombined) return;
    hasCombined = true;
    std::unique_lock lock(AllRecords::recordsLock);
    for (auto& [key, value] : otherRecords.items()) {
        AllRecords::records[key] = value;
    }
}
