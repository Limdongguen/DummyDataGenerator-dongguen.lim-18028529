#include "service/AppService.h"
#include <cmath>
#include <algorithm>

AppService::AppService(const std::string& dataDir)
    : dataDir_(dataDir), sampleStore_(dataDir), orderStore_(dataDir),
      inventoryStore_(dataDir), productionStore_(dataDir) {}

void AppService::reloadAllStores() {
    sampleStore_.reload();
    orderStore_.reload();
    inventoryStore_.reload();
    productionStore_.reload();
}

// ── 시료 ──────────────────────────────────────────────────
Sample AppService::registerSample(const std::string& name, double avgTime, double yield, int initStock) {
    Sample s{ sampleStore_.nextId(), name, avgTime, yield };
    sampleStore_.add(s);
    inventoryStore_.initStock(s.id, initStock);
    return s;
}
std::vector<Sample>   AppService::getAllSamples() const  { return sampleStore_.findAll(); }
std::optional<Sample> AppService::getSampleById(const std::string& id) const { return sampleStore_.findById(id); }
std::vector<Sample>   AppService::searchSamples(const std::string& kw) const { return sampleStore_.findByNameKeyword(kw); }
bool                  AppService::sampleExists(const std::string& id) const  { return sampleStore_.exists(id); }
int                   AppService::getSampleStock(const std::string& id) const { return inventoryStore_.getStock(id); }
std::map<std::string,int> AppService::getAllStock() const { return inventoryStore_.getAllStock(); }
int                   AppService::totalSamples() const   { return sampleStore_.count(); }

// ── 주문 ──────────────────────────────────────────────────
Order AppService::createOrder(const std::string& sampleId, const std::string& customer, int qty) {
    Order o{ orderStore_.generateOrderId(), sampleId, customer, qty, OrderStatus::RESERVED };
    orderStore_.add(o);
    return o;
}

ApprovalResult AppService::approveOrder(const std::string& orderId) {
    auto orderOpt = orderStore_.findById(orderId);
    if (!orderOpt || orderOpt->status != OrderStatus::RESERVED) return { false };
    auto sampleOpt = sampleStore_.findById(orderOpt->sampleId);
    if (!sampleOpt) return { false };

    const auto& sample = *sampleOpt;
    int stock = inventoryStore_.getStock(orderOpt->sampleId);
    int qty   = orderOpt->quantity;

    if (stock >= qty) {
        inventoryStore_.deductStock(orderOpt->sampleId, qty);
        orderStore_.updateStatus(orderId, OrderStatus::CONFIRMED);
        return { true, OrderStatus::CONFIRMED, stock, 0, 0, 0.0 };
    }
    int shortage   = qty - stock;
    int actualProd = static_cast<int>(std::ceil(shortage / (sample.yield * 0.9)));
    double totalTime = sample.avgProductionTime * actualProd;
    productionStore_.enqueue({ orderId, orderOpt->sampleId, qty, stock,
                               shortage, actualProd, totalTime, sample.yield });
    orderStore_.updateStatus(orderId, OrderStatus::PRODUCING);
    return { true, OrderStatus::PRODUCING, stock, shortage, actualProd, totalTime };
}

bool AppService::rejectOrder(const std::string& orderId) {
    auto o = orderStore_.findById(orderId);
    if (!o || o->status != OrderStatus::RESERVED) return false;
    return orderStore_.updateStatus(orderId, OrderStatus::REJECTED);
}
std::vector<Order> AppService::getReservedOrders() const    { return orderStore_.findByStatus(OrderStatus::RESERVED); }
std::vector<Order> AppService::getAllOrders() const          { return orderStore_.findAll(); }
int                AppService::totalOrdersExceptRejected() const { return orderStore_.countExceptRejected(); }

// ── 모니터링 ──────────────────────────────────────────────
OrderCounts AppService::getOrderCounts() const {
    return { orderStore_.countByStatus(OrderStatus::RESERVED),
             orderStore_.countByStatus(OrderStatus::PRODUCING),
             orderStore_.countByStatus(OrderStatus::CONFIRMED),
             orderStore_.countByStatus(OrderStatus::RELEASED) };
}
std::vector<InventoryInfo> AppService::getInventoryStatus() const {
    std::vector<InventoryInfo> result;
    auto producingOrders = orderStore_.findByStatus(OrderStatus::PRODUCING);
    for (const auto& s : sampleStore_.findAll()) {
        int stock = inventoryStore_.getStock(s.id);
        int pending = 0;
        for (const auto& o : producingOrders) if (o.sampleId == s.id) pending += o.quantity;
        std::string st = (stock == 0) ? "고갈" : (pending > 0 ? "부족" : "여유");
        result.push_back({ s.id, s.name, stock, pending, st });
    }
    return result;
}

// ── 생산라인 ──────────────────────────────────────────────
std::optional<ProductionJob> AppService::getCurrentJob() const  { return productionStore_.currentJob(); }
std::vector<ProductionJob>   AppService::getWaitingJobs() const { return productionStore_.waitingJobs(); }
bool AppService::hasProductionWork() const  { return productionStore_.hasWork(); }
int  AppService::productionQueueSize() const { return productionStore_.totalCount(); }

bool AppService::completeCurrentJob() {
    if (!productionStore_.hasWork()) return false;
    auto job      = productionStore_.dequeue();
    int goodItems = std::max(static_cast<int>(job.actualProduction * job.yield), job.shortage);
    inventoryStore_.addStock(job.sampleId, goodItems);
    inventoryStore_.deductStock(job.sampleId, job.orderQty);
    orderStore_.updateStatus(job.orderId, OrderStatus::CONFIRMED);
    return true;
}

// ── 출고 ──────────────────────────────────────────────────
std::vector<Order> AppService::getConfirmedOrders() const { return orderStore_.findByStatus(OrderStatus::CONFIRMED); }
bool AppService::release(const std::string& orderId) {
    auto o = orderStore_.findById(orderId);
    if (!o || o->status != OrderStatus::CONFIRMED) return false;
    return orderStore_.updateStatus(orderId, OrderStatus::RELEASED);
}

// ── 메인 메뉴 통계 ─────────────────────────────────────────
SystemStats AppService::getSystemStats() const {
    return { sampleStore_.count(), inventoryStore_.totalStock(),
             orderStore_.countExceptRejected(), productionStore_.totalCount() };
}
