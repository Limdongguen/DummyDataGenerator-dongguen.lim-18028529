#pragma once
#include <vector>
#include <optional>
#include <string>
#include "model/Models.h"

class OrderStore {
public:
    explicit OrderStore(const std::string& dataDir = "data");
    std::string generateOrderId();
    void add(const Order& order);
    std::vector<Order> findAll() const;
    std::optional<Order> findById(const std::string& orderId) const;
    std::vector<Order> findByStatus(OrderStatus status) const;
    bool updateStatus(const std::string& orderId, OrderStatus newStatus);
    int  countByStatus(OrderStatus status) const;
    int  countExceptRejected() const;
    void reload() { orders_.clear(); nextSeq_ = 1; load(); }
private:
    void load(); void save() const;
    std::string path_; std::vector<Order> orders_; int nextSeq_ = 1;
};
