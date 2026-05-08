#include "writer/JsonWriter.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <stdexcept>

JsonWriter::JsonWriter(const std::string& outDir) : outDir_(outDir) {
    std::filesystem::create_directories(outDir);
}

void JsonWriter::writeFile(const std::string& path, const std::string& content) const {
    std::ofstream f(path, std::ios::out | std::ios::trunc);
    if (!f.is_open()) throw std::runtime_error("파일 쓰기 실패: " + path);
    f << content;
}

void JsonWriter::writeSamples(const std::vector<Sample>& samples) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{\n  \"nextSeq\": " << (samples.size() + 1) << ",\n  \"samples\": [\n";
    for (size_t i = 0; i < samples.size(); ++i) {
        const auto& s = samples[i];
        oss << "    {\n"
            << "      \"id\": \""   << s.id   << "\",\n"
            << "      \"name\": \"" << s.name << "\",\n"
            << "      \"avgProductionTime\": " << s.avgProductionTime << ",\n"
            << "      \"yield\": "  << s.yield << "\n"
            << "    }" << (i + 1 < samples.size() ? "," : "") << "\n";
    }
    oss << "  ]\n}\n";
    writeFile(outDir_ + "/samples.json", oss.str());
}

void JsonWriter::writeOrders(const std::vector<Order>& orders) const {
    std::ostringstream oss;
    oss << "{\n  \"nextSeq\": " << (orders.size() + 1) << ",\n  \"orders\": [\n";
    for (size_t i = 0; i < orders.size(); ++i) {
        const auto& o = orders[i];
        oss << "    {\n"
            << "      \"orderId\": \""      << o.orderId      << "\",\n"
            << "      \"sampleId\": \""     << o.sampleId     << "\",\n"
            << "      \"customerName\": \"" << o.customerName << "\",\n"
            << "      \"quantity\": "       << o.quantity      << ",\n"
            << "      \"status\": \""       << statusToString(o.status) << "\"\n"
            << "    }" << (i + 1 < orders.size() ? "," : "") << "\n";
    }
    oss << "  ]\n}\n";
    writeFile(outDir_ + "/orders.json", oss.str());
}

void JsonWriter::writeInventory(const std::map<std::string,int>& inventory) const {
    std::ostringstream oss;
    oss << "{\n  \"inventory\": [\n";
    size_t i = 0;
    for (const auto& [id, qty] : inventory) {
        oss << "    {\n"
            << "      \"sampleId\": \"" << id  << "\",\n"
            << "      \"quantity\": "   << qty << "\n"
            << "    }" << (++i < inventory.size() ? "," : "") << "\n";
    }
    oss << "  ]\n}\n";
    writeFile(outDir_ + "/inventory.json", oss.str());
}

void JsonWriter::writeProduction(const std::vector<ProductionJob>& jobs) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{\n  \"jobs\": [\n";
    for (size_t i = 0; i < jobs.size(); ++i) {
        const auto& j = jobs[i];
        oss << "    {\n"
            << "      \"orderId\": \""       << j.orderId          << "\",\n"
            << "      \"sampleId\": \""      << j.sampleId         << "\",\n"
            << "      \"orderQty\": "        << j.orderQty         << ",\n"
            << "      \"currentStock\": "    << j.currentStock     << ",\n"
            << "      \"shortage\": "        << j.shortage         << ",\n"
            << "      \"actualProduction\": "<< j.actualProduction << ",\n"
            << "      \"totalTime\": "       << j.totalTime        << ",\n"
            << "      \"yield\": "           << j.yield            << "\n"
            << "    }" << (i + 1 < jobs.size() ? "," : "") << "\n";
    }
    oss << "  ]\n}\n";
    writeFile(outDir_ + "/production.json", oss.str());
}

void JsonWriter::write(const DummyDataSet& ds) const {
    writeSamples(ds.samples);
    writeOrders(ds.orders);
    writeInventory(ds.inventory);
    writeProduction(ds.productionJobs);
}
