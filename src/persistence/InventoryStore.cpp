#include "persistence/InventoryStore.h"
#include "persistence/JsonHelper.h"
#include <sstream>

InventoryStore::InventoryStore(const std::string& dataDir) {
    json::ensureDataDir(dataDir);
    path_ = dataDir + "/inventory.json";
    load();
}

void InventoryStore::load() {
    std::string content = json::readFile(path_);
    if (content.empty()) return;
    for (const auto& obj : json::extractObjects(content, "inventory")) {
        std::string id = json::extractStr(obj, "sampleId");
        int qty        = json::extractInt(obj, "quantity");
        if (!id.empty()) stock_[id] = qty;
    }
}

void InventoryStore::save() const {
    std::ostringstream oss;
    oss << "{\n  \"inventory\": [\n";
    size_t i = 0;
    for (const auto& [id, qty] : stock_) {
        oss << "    {\n"
            << "      \"sampleId\": \"" << id  << "\",\n"
            << "      \"quantity\": "   << qty << "\n"
            << "    }" << (++i < stock_.size() ? "," : "") << "\n";
    }
    oss << "  ]\n}\n";
    json::writeFile(path_, oss.str());
}

void InventoryStore::initStock(const std::string& sampleId, int quantity) {
    stock_[sampleId] = quantity;
    save();
}

int InventoryStore::getStock(const std::string& sampleId) const {
    auto it = stock_.find(sampleId);
    return it != stock_.end() ? it->second : 0;
}

void InventoryStore::addStock(const std::string& sampleId, int quantity) {
    stock_[sampleId] += quantity;
    save();
}

bool InventoryStore::deductStock(const std::string& sampleId, int quantity) {
    auto it = stock_.find(sampleId);
    if (it == stock_.end() || it->second < quantity) return false;
    it->second -= quantity;
    save();
    return true;
}

std::map<std::string, int> InventoryStore::getAllStock() const { return stock_; }

int InventoryStore::totalStock() const {
    int total = 0;
    for (const auto& [id, qty] : stock_) total += qty;
    return total;
}
