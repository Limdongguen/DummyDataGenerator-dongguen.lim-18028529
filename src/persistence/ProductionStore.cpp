#include "persistence/ProductionStore.h"
#include "persistence/JsonHelper.h"
#include <sstream>
#include <iomanip>

ProductionStore::ProductionStore(const std::string& dataDir) {
    json::ensureDataDir(dataDir);
    path_ = dataDir + "/production.json";
    load();
}

void ProductionStore::load() {
    std::string content = json::readFile(path_);
    if (content.empty()) return;
    for (const auto& obj : json::extractObjects(content, "jobs")) {
        ProductionJob j;
        j.orderId          = json::extractStr(obj, "orderId");
        j.sampleId         = json::extractStr(obj, "sampleId");
        j.orderQty         = json::extractInt(obj, "orderQty");
        j.currentStock     = json::extractInt(obj, "currentStock");
        j.shortage         = json::extractInt(obj, "shortage");
        j.actualProduction = json::extractInt(obj, "actualProduction");
        j.totalTime        = json::extractDbl(obj, "totalTime");
        j.yield            = json::extractDbl(obj, "yield");
        if (!j.orderId.empty()) jobs_.push_back(j);
    }
}

void ProductionStore::save() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{\n  \"jobs\": [\n";
    for (size_t i = 0; i < jobs_.size(); ++i) {
        const auto& j = jobs_[i];
        oss << "    {\n"
            << "      \"orderId\": \""      << j.orderId          << "\",\n"
            << "      \"sampleId\": \""     << j.sampleId         << "\",\n"
            << "      \"orderQty\": "       << j.orderQty         << ",\n"
            << "      \"currentStock\": "   << j.currentStock     << ",\n"
            << "      \"shortage\": "       << j.shortage         << ",\n"
            << "      \"actualProduction\": "<< j.actualProduction << ",\n"
            << "      \"totalTime\": "      << j.totalTime        << ",\n"
            << "      \"yield\": "          << j.yield            << "\n"
            << "    }" << (i + 1 < jobs_.size() ? "," : "") << "\n";
    }
    oss << "  ]\n}\n";
    json::writeFile(path_, oss.str());
}

void ProductionStore::enqueue(const ProductionJob& job) {
    jobs_.push_back(job);
    save();
}

bool ProductionStore::hasWork() const { return !jobs_.empty(); }

std::optional<ProductionJob> ProductionStore::currentJob() const {
    if (jobs_.empty()) return std::nullopt;
    return jobs_.front();
}

std::vector<ProductionJob> ProductionStore::waitingJobs() const {
    if (jobs_.size() <= 1) return {};
    return std::vector<ProductionJob>(jobs_.begin() + 1, jobs_.end());
}

ProductionJob ProductionStore::dequeue() {
    ProductionJob j = jobs_.front();
    jobs_.pop_front();
    save();
    return j;
}

int ProductionStore::totalCount() const { return static_cast<int>(jobs_.size()); }
