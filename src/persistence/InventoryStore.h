#pragma once
#include <string>
#include <map>

class InventoryStore {
public:
    explicit InventoryStore(const std::string& dataDir = "data");
    void initStock(const std::string& sampleId, int quantity = 0);
    int  getStock(const std::string& sampleId) const;
    void addStock(const std::string& sampleId, int quantity);
    bool deductStock(const std::string& sampleId, int quantity);
    std::map<std::string, int> getAllStock() const;
    int  totalStock() const;
    void reload() { stock_.clear(); load(); }
private:
    void load(); void save() const;
    std::string path_; std::map<std::string, int> stock_;
};
