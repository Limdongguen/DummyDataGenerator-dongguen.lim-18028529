#include "reader/DataReader.h"
#include "persistence/JsonHelper.h"
#include <ctime>

static std::string nowStr() {
    std::time_t t = std::time(nullptr);
    struct tm tm_buf; localtime_s(&tm_buf, &t);
    char buf[32]; std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
    return buf;
}

DataReader::DataReader(const std::string& dataDir) : dataDir_(dataDir) {}

DataSnapshot DataReader::load() const {
    DataSnapshot snap;
    snap.dataDir = dataDir_;
    snap.loadTime = nowStr();

    // ── 시료 ──────────────────────────────────────────────
    auto sJson = json::readFile(dataDir_ + "/samples.json");
    for (const auto& obj : json::extractObjects(sJson, "samples")) {
        Sample s;
        s.id                = json::extractStr(obj, "id");
        s.name              = json::extractStr(obj, "name");
        s.avgProductionTime = json::extractDbl(obj, "avgProductionTime");
        s.yield             = json::extractDbl(obj, "yield");
        if (!s.id.empty()) snap.samples.push_back(s);
    }

    // ── 주문 ──────────────────────────────────────────────
    auto oJson = json::readFile(dataDir_ + "/orders.json");
    for (const auto& obj : json::extractObjects(oJson, "orders")) {
        Order o;
        o.orderId      = json::extractStr(obj, "orderId");
        o.sampleId     = json::extractStr(obj, "sampleId");
        o.customerName = json::extractStr(obj, "customerName");
        o.quantity     = json::extractInt(obj, "quantity");
        o.status       = statusFromString(json::extractStr(obj, "status"));
        if (!o.orderId.empty()) snap.orders.push_back(o);
    }

    // ── 재고 ──────────────────────────────────────────────
    auto iJson = json::readFile(dataDir_ + "/inventory.json");
    for (const auto& obj : json::extractObjects(iJson, "inventory")) {
        std::string id = json::extractStr(obj, "sampleId");
        int qty        = json::extractInt(obj, "quantity");
        if (!id.empty()) snap.inventory[id] = qty;
    }

    // ── 생산라인 ──────────────────────────────────────────
    auto pJson = json::readFile(dataDir_ + "/production.json");
    for (const auto& obj : json::extractObjects(pJson, "jobs")) {
        ProductionJob j;
        j.orderId          = json::extractStr(obj, "orderId");
        j.sampleId         = json::extractStr(obj, "sampleId");
        j.orderQty         = json::extractInt(obj, "orderQty");
        j.currentStock     = json::extractInt(obj, "currentStock");
        j.shortage         = json::extractInt(obj, "shortage");
        j.actualProduction = json::extractInt(obj, "actualProduction");
        j.totalTime        = json::extractDbl(obj, "totalTime");
        j.yield            = json::extractDbl(obj, "yield");
        if (!j.orderId.empty()) snap.productionJobs.push_back(j);
    }

    snap.valid = !snap.samples.empty() || !snap.orders.empty();
    return snap;
}
