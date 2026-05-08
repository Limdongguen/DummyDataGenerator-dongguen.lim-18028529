#include "generator/DataGenerator.h"
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>

// ── 사전 정의 데이터 ──────────────────────────────────────
const std::vector<std::pair<std::string,double>> DataGenerator::SAMPLE_TEMPLATES = {
    {"실리콘 웨이퍼-8인치",  0.92},
    {"GaN 에피택셜-4인치",   0.78},
    {"SiC 파워기판-6인치",   0.92},
    {"포토레지스트-PR7",      0.95},
    {"산화막 웨이퍼-SiO2",   0.88},
    {"InP 에피택셜-3인치",   0.75},
    {"GaAs 기판-4인치",      0.82},
    {"실리콘 웨이퍼-12인치", 0.90},
    {"AlGaN 에피택셜",       0.70},
    {"HfO2 게이트 산화막",   0.85},
};
const std::vector<double> DataGenerator::AVG_TIMES = {
    0.5, 0.3, 0.8, 0.2, 0.6, 1.0, 0.7, 0.4, 0.9, 0.35
};
const std::vector<std::string> DataGenerator::CUSTOMERS = {
    "LG이노텍", "SK하이닉스", "삼성전자파운드리",
    "DB하이텍", "매그나칩", "SK실트론", "주성엔지니어링"
};
const std::vector<OrderStatus> DataGenerator::STATUS_POOL = {
    OrderStatus::RESERVED, OrderStatus::CONFIRMED, OrderStatus::CONFIRMED,
    OrderStatus::PRODUCING, OrderStatus::RELEASED, OrderStatus::RELEASED
};

// ── 생성자 ────────────────────────────────────────────────
DataGenerator::DataGenerator(unsigned int seed)
    : seed_(seed == 0 ? static_cast<unsigned int>(std::time(nullptr)) : seed) {}

unsigned int DataGenerator::nextRand() {
    seed_ = seed_ * 1664525u + 1013904223u; // LCG
    return seed_;
}

// ── 시료 생성 ─────────────────────────────────────────────
Sample DataGenerator::makeSample(int seq) {
    size_t idx = (seq - 1) % SAMPLE_TEMPLATES.size();
    double avgT = AVG_TIMES[idx % AVG_TIMES.size()];

    std::ostringstream id;
    id << "S-" << std::setw(3) << std::setfill('0') << seq;

    return { id.str(), SAMPLE_TEMPLATES[idx].first, avgT, SAMPLE_TEMPLATES[idx].second };
}

// ── 주문 생성 ─────────────────────────────────────────────
Order DataGenerator::makeOrder(int seq, const std::vector<Sample>& samples) {
    auto t = std::time(nullptr);
    struct tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    std::ostringstream oid;
    oid << "ORD-" << std::put_time(&tm_buf, "%Y%m%d")
        << "-" << std::setw(4) << std::setfill('0') << seq;

    const auto& sample = samples[(nextRand() % samples.size())];
    const auto& customer = CUSTOMERS[nextRand() % CUSTOMERS.size()];
    int qty = 50 + static_cast<int>(nextRand() % 451); // 50~500
    OrderStatus st = STATUS_POOL[nextRand() % STATUS_POOL.size()];

    return { oid.str(), sample.id, customer, qty, st };
}

// ── 재고 + 생산 큐 생성 ───────────────────────────────────
void DataGenerator::buildInventoryAndJobs(DummyDataSet& ds) {
    // 시료별 초기 재고 설정
    for (const auto& s : ds.samples) {
        int stock = 50 + static_cast<int>(nextRand() % 951); // 50~1000
        ds.inventory[s.id] = stock;
    }

    // PRODUCING 주문에 대해 생산 큐 생성
    for (const auto& o : ds.orders) {
        if (o.status != OrderStatus::PRODUCING) continue;

        int stock = ds.inventory.count(o.sampleId) ? ds.inventory.at(o.sampleId) : 0;
        int shortage = (o.quantity > stock) ? o.quantity - stock : 1;

        double yield = 0.9;
        for (const auto& s : ds.samples)
            if (s.id == o.sampleId) { yield = s.yield; break; }

        int actualProd = static_cast<int>(std::ceil(shortage / (yield * 0.9)));
        double totalTime = 0.5 * actualProd; // 기본 생산시간으로 계산

        ds.productionJobs.push_back({
            o.orderId, o.sampleId, o.quantity,
            stock, shortage, actualProd, totalTime, yield
        });
    }
}

// ── 메인 생성 함수 ────────────────────────────────────────
DummyDataSet DataGenerator::generate(int sampleCount, int orderCount) {
    sampleSeq_ = 1; orderSeq_ = 1;
    DummyDataSet ds;

    int sc = std::min(sampleCount, static_cast<int>(SAMPLE_TEMPLATES.size()));
    for (int i = 0; i < sc; ++i)
        ds.samples.push_back(makeSample(sampleSeq_++));

    for (int i = 0; i < orderCount; ++i)
        ds.orders.push_back(makeOrder(orderSeq_++, ds.samples));

    buildInventoryAndJobs(ds);
    return ds;
}
