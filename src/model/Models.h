#pragma once
#include <string>
#include <vector>
#include <map>

enum class OrderStatus { RESERVED, REJECTED, PRODUCING, CONFIRMED, RELEASED };

inline std::string statusToString(OrderStatus s) {
    switch (s) {
    case OrderStatus::RESERVED:  return "RESERVED";
    case OrderStatus::REJECTED:  return "REJECTED";
    case OrderStatus::PRODUCING: return "PRODUCING";
    case OrderStatus::CONFIRMED: return "CONFIRMED";
    case OrderStatus::RELEASED:  return "RELEASED";
    default: return "UNKNOWN";
    }
}
inline OrderStatus statusFromString(const std::string& s) {
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "RELEASED")  return OrderStatus::RELEASED;
    return OrderStatus::RESERVED;
}

struct Sample {
    std::string id, name;
    double avgProductionTime = 0.0;
    double yield = 0.0;
};
struct Order {
    std::string orderId, sampleId, customerName;
    int quantity = 0;
    OrderStatus status = OrderStatus::RESERVED;
};
struct ProductionJob {
    std::string orderId, sampleId;
    int orderQty = 0, currentStock = 0, shortage = 0, actualProduction = 0;
    double totalTime = 0.0, yield = 0.0;
};

// CRUD / 서비스 레이어 DTO
struct ApprovalResult {
    bool success = false;
    OrderStatus newStatus = OrderStatus::RESERVED;
    int    currentStock = 0;
    int    shortage = 0;
    int    actualProduction = 0;
    double totalTime = 0.0;
};

struct SystemStats {
    int sampleCount = 0;
    int totalStock = 0;
    int orderCount = 0;
    int productionQueueSize = 0;
};

struct OrderCounts {
    int reserved = 0, producing = 0, confirmed = 0, released = 0;
};

struct InventoryInfo {
    std::string sampleId;
    std::string sampleName;
    int stock = 0;
    int totalPendingQty = 0;
    std::string stockStatus; // "여유" | "부족" | "고갈"
};

// 더미 데이터 생성 결과 세트
struct DummyDataSet {
    std::vector<Sample>        samples;
    std::vector<Order>         orders;
    std::map<std::string, int> inventory;
    std::vector<ProductionJob> productionJobs;
};

// 모니터링용 스냅샷 — JSON 파일에서 한 번에 로드한 전체 상태
struct DataSnapshot {
    std::vector<Sample>            samples;
    std::vector<Order>             orders;
    std::map<std::string, int>     inventory;
    std::vector<ProductionJob>     productionJobs;
    std::string                    loadTime;
    std::string                    dataDir;
    bool                           valid = false;  // 파일이 존재하면 true
};
