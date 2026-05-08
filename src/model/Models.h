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

struct Sample {
    std::string id;
    std::string name;
    double avgProductionTime = 0.0;
    double yield = 0.0;
};

struct Order {
    std::string orderId;
    std::string sampleId;
    std::string customerName;
    int quantity = 0;
    OrderStatus status = OrderStatus::RESERVED;
};

struct ProductionJob {
    std::string orderId;
    std::string sampleId;
    int orderQty = 0, currentStock = 0, shortage = 0, actualProduction = 0;
    double totalTime = 0.0, yield = 0.0;
};

struct DummyDataSet {
    std::vector<Sample>       samples;
    std::vector<Order>        orders;
    std::map<std::string,int> inventory;
    std::vector<ProductionJob>productionJobs;
};
