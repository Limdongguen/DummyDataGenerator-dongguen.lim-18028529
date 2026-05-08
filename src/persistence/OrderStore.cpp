#include "persistence/OrderStore.h"
#include "persistence/JsonHelper.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

OrderStore::OrderStore(const std::string& dataDir) {
    json::ensureDataDir(dataDir);
    path_ = dataDir + "/orders.json";
    load();
}

void OrderStore::load() {
    std::string content = json::readFile(path_);
    if (content.empty()) return;
    nextSeq_ = json::extractInt(content, "nextSeq");
    if (nextSeq_ < 1) nextSeq_ = 1;
    for (const auto& obj : json::extractObjects(content, "orders")) {
        Order o;
        o.orderId      = json::extractStr(obj, "orderId");
        o.sampleId     = json::extractStr(obj, "sampleId");
        o.customerName = json::extractStr(obj, "customerName");
        o.quantity     = json::extractInt(obj, "quantity");
        o.status       = statusFromString(json::extractStr(obj, "status"));
        if (!o.orderId.empty()) orders_.push_back(o);
    }
}

void OrderStore::save() const {
    std::ostringstream oss;
    oss << "{\n  \"nextSeq\": " << nextSeq_ << ",\n  \"orders\": [\n";
    for (size_t i = 0; i < orders_.size(); ++i) {
        const auto& o = orders_[i];
        oss << "    {\n"
            << "      \"orderId\": \""      << o.orderId      << "\",\n"
            << "      \"sampleId\": \""     << o.sampleId     << "\",\n"
            << "      \"customerName\": \"" << o.customerName << "\",\n"
            << "      \"quantity\": "       << o.quantity      << ",\n"
            << "      \"status\": \""       << statusToString(o.status) << "\"\n"
            << "    }" << (i + 1 < orders_.size() ? "," : "") << "\n";
    }
    oss << "  ]\n}\n";
    json::writeFile(path_, oss.str());
}

std::string OrderStore::generateOrderId() {
    auto t = std::time(nullptr);
    struct tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    std::ostringstream oss;
    oss << "ORD-" << std::put_time(&tm_buf, "%Y%m%d")
        << "-" << std::setw(4) << std::setfill('0') << nextSeq_++;
    return oss.str();
}

void OrderStore::add(const Order& order) {
    orders_.push_back(order);
    save();
}

std::vector<Order> OrderStore::findAll() const { return orders_; }

std::optional<Order> OrderStore::findById(const std::string& orderId) const {
    for (const auto& o : orders_) if (o.orderId == orderId) return o;
    return std::nullopt;
}

std::vector<Order> OrderStore::findByStatus(OrderStatus status) const {
    std::vector<Order> res;
    for (const auto& o : orders_) if (o.status == status) res.push_back(o);
    return res;
}

bool OrderStore::updateStatus(const std::string& orderId, OrderStatus newStatus) {
    for (auto& o : orders_) {
        if (o.orderId == orderId) { o.status = newStatus; save(); return true; }
    }
    return false;
}

int OrderStore::countByStatus(OrderStatus status) const {
    int n = 0;
    for (const auto& o : orders_) if (o.status == status) ++n;
    return n;
}

int OrderStore::countExceptRejected() const {
    int n = 0;
    for (const auto& o : orders_) if (o.status != OrderStatus::REJECTED) ++n;
    return n;
}
