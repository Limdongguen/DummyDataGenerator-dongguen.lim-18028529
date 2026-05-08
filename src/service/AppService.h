#pragma once
#include <vector>
#include <optional>
#include <string>
#include <map>
#include "model/Models.h"
#include "persistence/SampleStore.h"
#include "persistence/OrderStore.h"
#include "persistence/InventoryStore.h"
#include "persistence/ProductionStore.h"

// 시료·주문·모니터링·생산·출고 비즈니스 로직을 하나의 클래스로 통합
class AppService {
public:
    explicit AppService(const std::string& dataDir = "data");

    // ── 시료 ─────────────────────────────────────────────
    Sample              registerSample(const std::string& name, double avgTime, double yield, int initStock = 0);
    std::vector<Sample> getAllSamples() const;
    std::optional<Sample> getSampleById(const std::string& id) const;
    std::vector<Sample> searchSamples(const std::string& keyword) const;
    bool                sampleExists(const std::string& id) const;
    int                 getSampleStock(const std::string& id) const;
    std::map<std::string,int> getAllStock() const;
    int                 totalSamples() const;

    // ── 주문 ─────────────────────────────────────────────
    Order          createOrder(const std::string& sampleId, const std::string& customer, int qty);
    ApprovalResult approveOrder(const std::string& orderId);
    bool           rejectOrder(const std::string& orderId);
    std::vector<Order> getReservedOrders() const;
    std::vector<Order> getAllOrders() const;
    int            totalOrdersExceptRejected() const;

    // ── 모니터링 ──────────────────────────────────────────
    OrderCounts                getOrderCounts() const;
    std::vector<InventoryInfo> getInventoryStatus() const;

    // ── 생산라인 ──────────────────────────────────────────
    std::optional<ProductionJob> getCurrentJob() const;
    std::vector<ProductionJob>   getWaitingJobs() const;
    bool completeCurrentJob();
    bool hasProductionWork() const;
    int  productionQueueSize() const;

    // ── 출고 ─────────────────────────────────────────────
    std::vector<Order> getConfirmedOrders() const;
    bool               release(const std::string& orderId);

    // ── 메인 메뉴 통계 ────────────────────────────────────
    SystemStats getSystemStats() const;

    // ── 더미 데이터 ──────────────────────────────────────
    void reloadAllStores();
    const std::string& dataDir() const { return dataDir_; }

private:
    std::string     dataDir_;
    SampleStore     sampleStore_;
    OrderStore      orderStore_;
    InventoryStore  inventoryStore_;
    ProductionStore productionStore_;
};
