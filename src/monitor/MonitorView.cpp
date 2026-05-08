#include "monitor/MonitorView.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>

namespace {
    const char* R  = "\033[0m";
    const char* B  = "\033[1m";
    const char* CY = "\033[36m";
    const char* GR = "\033[32m";
    const char* RD = "\033[31m";
    const char* YL = "\033[33m";
    const char* MG = "\033[35m";
    const char* BL = "\033[34m";

    std::string sampleName(const std::string& id, const std::vector<Sample>& samples) {
        for (const auto& s : samples) if (s.id == id) return s.name;
        return id;
    }
    int totalStock(const std::map<std::string,int>& inv) {
        int t = 0; for (const auto& [k,v] : inv) t += v; return t;
    }
    int countByStatus(const std::vector<Order>& orders, OrderStatus st) {
        return (int)std::count_if(orders.begin(), orders.end(),
            [st](const Order& o){ return o.status == st; });
    }
}

void MonitorView::clearScreen() { std::cout << "\033[2J\033[H"; }

void MonitorView::line(char c, int w) {
    for (int i = 0; i < w; ++i) std::cout << c; std::cout << '\n';
}

std::string MonitorView::colored(const std::string& s) {
    if (s=="RESERVED")  return std::string(CY)+s+R;
    if (s=="REJECTED")  return std::string(RD)+s+R;
    if (s=="PRODUCING") return std::string(YL)+s+R;
    if (s=="CONFIRMED") return std::string(GR)+s+R;
    if (s=="RELEASED")  return std::string(MG)+s+R;
    return s;
}

std::string MonitorView::stockBar(int val, int maxVal, int width) {
    int filled = (maxVal > 0) ? val * width / maxVal : 0;
    filled = std::min(filled, width);
    return std::string(GR) + std::string(filled, '#') + R + std::string(width - filled, '.');
}

// ── 대시보드 (자동 갱신) ──────────────────────────────────
void MonitorView::printDashboard(const DataSnapshot& snap, int countdown) const {
    int reserved  = countByStatus(snap.orders, OrderStatus::RESERVED);
    int producing = countByStatus(snap.orders, OrderStatus::PRODUCING);
    int confirmed = countByStatus(snap.orders, OrderStatus::CONFIRMED);
    int released  = countByStatus(snap.orders, OrderStatus::RELEASED);
    int total     = reserved + producing + confirmed + released;
    int stock     = totalStock(snap.inventory);

    line();
    std::cout << B << CY << "  DataMonitor — 반도체 시료 생산주문관리 모니터링\n" << R;
    line();
    std::cout << "  갱신 시각: " << snap.loadTime
              << "   경로: " << snap.dataDir << "\n";
    std::cout << "  자동 갱신: " << countdown << "초 후   "
              << RD << "[Q]" << R << " 종료  "
              << CY << "[R]" << R << " 즉시 갱신  "
              << CY << "[M]" << R << " 메뉴\n";
    line('-');

    // 시스템 현황
    std::cout << B << "  시스템 현황\n" << R;
    std::cout << "  등록 시료  " << YL << snap.samples.size() << "종" << R
              << "     총 재고   " << GR << stock << " ea" << R
              << "     전체 주문  " << YL << total << "건" << R
              << "     생산라인  " << CY << snap.productionJobs.size() << "건" << R << "\n";
    line('-');

    // 주문 현황
    std::cout << B << "  주문 상태별 현황\n" << R;
    auto bar = [](int n, int total, int w=20) -> std::string {
        int f = (total>0) ? n*w/total : 0;
        return std::string(f,'#') + std::string(w-f,'-');
    };
    std::cout << "  " << colored("RESERVED")  << "  " << std::setw(3) << reserved
              << "건  " << GR << bar(reserved,  total) << R << "\n";
    std::cout << "  " << colored("PRODUCING") << " " << std::setw(3) << producing
              << "건  " << YL << bar(producing, total) << R << "\n";
    std::cout << "  " << colored("CONFIRMED") << " " << std::setw(3) << confirmed
              << "건  " << GR << bar(confirmed, total) << R << "\n";
    std::cout << "  " << colored("RELEASED")  << "  " << std::setw(3) << released
              << "건  " << MG << bar(released,  total) << R << "\n";
    line('-');

    // 재고 현황
    std::cout << B << "  재고 현황\n" << R;
    if (snap.samples.empty()) { std::cout << "  (데이터 없음)\n"; }
    else {
        int maxStock = 1;
        for (const auto& s : snap.samples) {
            int q = snap.inventory.count(s.id) ? snap.inventory.at(s.id) : 0;
            maxStock = std::max(maxStock, q);
        }
        for (const auto& s : snap.samples) {
            int qty  = snap.inventory.count(s.id) ? snap.inventory.at(s.id) : 0;
            int pct  = maxStock > 0 ? qty * 100 / maxStock : 0;
            bool hasProd = std::any_of(snap.productionJobs.begin(), snap.productionJobs.end(),
                [&](const ProductionJob& j){ return j.sampleId == s.id; });
            std::string st = (qty==0) ? std::string(RD)+"고갈"+R
                           : hasProd  ? std::string(YL)+"부족"+R
                                       : std::string(GR)+"여유"+R;
            std::cout << "  " << std::left << std::setw(8) << s.id
                      << std::setw(20) << s.name
                      << "[" << stockBar(qty, maxStock) << "] "
                      << std::setw(6) << qty << "ea  " << st << "\n";
        }
    }
    line('-');

    // 생산라인 요약
    std::cout << B << "  생산라인\n" << R;
    if (snap.productionJobs.empty()) {
        std::cout << "  " << GR << "IDLE" << R << " — 대기 중인 작업 없음\n";
    } else {
        const auto& cur = snap.productionJobs[0];
        std::cout << "  " << GR << "RUNNING" << R
                  << "  주문 " << cur.orderId
                  << "  시료 " << sampleName(cur.sampleId, snap.samples)
                  << "  실생산량 " << cur.actualProduction << " ea\n";
        if (snap.productionJobs.size() > 1)
            std::cout << "  대기 " << (snap.productionJobs.size()-1) << "건\n";
    }
    line('-');
}

// ── 메인 메뉴 ─────────────────────────────────────────────
void MonitorView::printMenu(const std::string& dataDir) const {
    line();
    std::cout << B << CY << "  DataMonitor — 반도체 시료 생산주문관리 모니터링\n" << R;
    std::cout << "  데이터 경로: " << dataDir << "\n";
    line('-');
    std::cout << "  " << CY << "[1]" << R << " 대시보드 (자동 갱신)\n"
              << "  " << CY << "[2]" << R << " 주문 목록 상세\n"
              << "  " << CY << "[3]" << R << " 시료 및 재고 현황\n"
              << "  " << CY << "[4]" << R << " 생산라인 현황\n"
              << "  " << RD << "[0]" << R << " 종료\n";
    line('-');
    std::cout << "  선택 > ";
}

// ── 주문 목록 상세 ────────────────────────────────────────
void MonitorView::printOrderList(const DataSnapshot& snap) const {
    line();
    std::cout << B << "  주문 목록 상세   " << snap.loadTime << R << "\n";
    line('-');
    if (snap.orders.empty()) { std::cout << "  (주문 없음)\n"; line('-'); return; }
    std::cout << CY << std::left
              << "  " << std::setw(22) << "주문번호"
              << std::setw(10) << "시료ID"
              << std::setw(14) << "고객"
              << std::setw(8)  << "수량"
              << "상태\n" << R; line('-');
    for (const auto& o : snap.orders) {
        std::cout << "  " << std::left
                  << std::setw(22) << o.orderId
                  << std::setw(10) << o.sampleId
                  << std::setw(14) << o.customerName
                  << std::setw(8)  << (std::to_string(o.quantity)+" ea")
                  << colored(statusToString(o.status)) << "\n";
    }
    line('-');
    std::cout << "  총 " << snap.orders.size() << "건\n";
    line('-');
}

// ── 시료 및 재고 현황 ─────────────────────────────────────
void MonitorView::printInventoryDetail(const DataSnapshot& snap) const {
    line();
    std::cout << B << "  시료 및 재고 현황   " << snap.loadTime << R << "\n";
    line('-');
    if (snap.samples.empty()) { std::cout << "  (시료 없음)\n"; line('-'); return; }
    int maxStock = 1;
    for (const auto& s : snap.samples) {
        int q = snap.inventory.count(s.id) ? snap.inventory.at(s.id) : 0;
        maxStock = std::max(maxStock, q);
    }
    std::cout << CY << std::left
              << "  " << std::setw(8) << "ID"
              << std::setw(22) << "시료명"
              << std::setw(14) << "생산시간"
              << std::setw(8)  << "수율"
              << std::setw(8)  << "재고"
              << "상태\n" << R; line('-');
    for (const auto& s : snap.samples) {
        int qty = snap.inventory.count(s.id) ? snap.inventory.at(s.id) : 0;
        bool hasProd = std::any_of(snap.productionJobs.begin(), snap.productionJobs.end(),
            [&](const ProductionJob& j){ return j.sampleId == s.id; });
        std::string st = (qty==0) ? std::string(RD)+"고갈"+R
                       : hasProd  ? std::string(YL)+"부족"+R
                                   : std::string(GR)+"여유"+R;
        std::cout << "  " << std::left
                  << std::setw(8)  << s.id
                  << std::setw(22) << s.name
                  << std::setw(14) << (std::to_string(s.avgProductionTime)+" min/ea")
                  << std::setw(8)  << s.yield
                  << std::setw(8)  << (std::to_string(qty)+" ea")
                  << st << "\n";
    }
    line('-');
    std::cout << "  총 재고: " << GR << totalStock(snap.inventory) << " ea" << R << "\n";
    line('-');
}

// ── 생산라인 현황 ─────────────────────────────────────────
void MonitorView::printProductionDetail(const DataSnapshot& snap) const {
    line();
    std::cout << B << "  생산라인 현황   " << snap.loadTime << R << "\n";
    line('-');
    if (snap.productionJobs.empty()) {
        std::cout << "  " << GR << "IDLE" << R << " — 생산 중인 작업 없음\n"; line('-'); return;
    }
    int idx = 0;
    for (const auto& j : snap.productionJobs) {
        std::string tag = (idx==0) ? std::string(GR)+"[현재]"+R : "  ["+std::to_string(idx)+"]";
        std::cout << "  " << tag << "\n"
                  << "    주문번호   " << j.orderId << "\n"
                  << "    시료       " << sampleName(j.sampleId, snap.samples) << "\n"
                  << "    주문량     " << j.orderQty << " ea"
                  << "   재고 " << j.currentStock << " ea"
                  << " → 부족 " << j.shortage << " ea\n"
                  << "    실생산량   " << j.actualProduction
                  << " ea  (수율 " << j.yield << " / " << (int)j.totalTime << " min)\n";
        if (idx==0 && snap.productionJobs.size()>1) { line('-'); std::cout << "  대기 목록\n"; }
        ++idx;
    }
    line('-');
}

void MonitorView::printError(const std::string& msg) const {
    std::cout << "  " << RD << "오류: " << msg << R << "\n";
}
