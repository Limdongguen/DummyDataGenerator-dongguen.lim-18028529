#pragma once
#include <string>
#include "model/Models.h"

// 더미 데이터를 DataPersistence 호환 JSON 파일로 기록
class JsonWriter {
public:
    explicit JsonWriter(const std::string& outDir = "data");
    void write(const DummyDataSet& ds) const;
    const std::string& outDir() const { return outDir_; }
private:
    void writeSamples(const std::vector<Sample>& samples) const;
    void writeOrders(const std::vector<Order>& orders) const;
    void writeInventory(const std::map<std::string,int>& inventory) const;
    void writeProduction(const std::vector<ProductionJob>& jobs) const;
    void writeFile(const std::string& path, const std::string& content) const;
    std::string outDir_;
};
