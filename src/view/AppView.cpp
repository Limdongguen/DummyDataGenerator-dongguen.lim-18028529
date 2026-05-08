#include "view/AppView.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <algorithm>

namespace {
    const char* R  = "\033[0m";
    const char* B  = "\033[1m";
    const char* CY = "\033[36m";
    const char* GR = "\033[32m";
    const char* RD = "\033[31m";
    const char* YL = "\033[33m";
    const char* MG = "\033[35m";

    std::string nowStr() {
        std::time_t t = std::time(nullptr);
        struct tm tm_buf; localtime_s(&tm_buf, &t);
        char buf[32]; std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
        return buf;
    }
    std::string sampleName(const std::string& id, const std::vector<Sample>& samples) {
        for (const auto& s : samples) if (s.id == id) return s.name;
        return id;
    }
}

// ── 공통 ──────────────────────────────────────────────────
void AppView::line(char c, int w) { for(int i=0;i<w;++i) std::cout<<c; std::cout<<'\n'; }

std::string AppView::colored(const std::string& s) {
    if (s=="RESERVED")  return std::string(CY)+s+R;
    if (s=="REJECTED")  return std::string(RD)+s+R;
    if (s=="PRODUCING") return std::string(YL)+s+R;
    if (s=="CONFIRMED") return std::string(GR)+s+R;
    if (s=="RELEASED")  return std::string(MG)+s+R;
    return s;
}

void AppView::printError(const std::string& msg) const {
    std::cout << "  " << RD << "오류: " << msg << R << "\n";
}
void AppView::printSuccess(const std::string& msg) const {
    std::cout << GR << "\n  " << msg << R << "\n";
}

// ── 메인 메뉴 ─────────────────────────────────────────────
void AppView::printMainMenu(const SystemStats& stats) const {
    line(); std::cout << B<<CY<<"        반도체 시료 생산주문관리 시스템\n"<<R; line();
    std::cout << "  시스템 현황   " << nowStr() << "\n\n";
    std::cout << "  등록 시료  " << YL << std::setw(4) << stats.sampleCount << "종" << R
              << "       총 재고   " << GR << stats.totalStock << " ea" << R << "\n";
    std::cout << "  전체 주문  " << YL << std::setw(4) << stats.orderCount << "건" << R
              << "       생산라인  " << CY << stats.productionQueueSize << "건 대기" << R << "\n";
    line('-');
    std::cout << "\n"
              << "  "<<CY<<"[1]"<<R<<" 시료 관리          "<<CY<<"[2]"<<R<<" 시료 주문\n"
              << "  "<<CY<<"[3]"<<R<<" 주문 승인/거절      "<<CY<<"[4]"<<R<<" 모니터링 (실시간)\n"
              << "  "<<CY<<"[5]"<<R<<" 생산라인 조회      "<<CY<<"[6]"<<R<<" 출고 처리\n"
              << "  "<<YL<<"[7]"<<R<<" 더미 데이터 생성\n"
              << "  "<<RD<<"[0]"<<R<<" 종료\n";
    line('-'); std::cout << "  선택 > ";
}

// ── 시료 관리 ─────────────────────────────────────────────
void AppView::printSampleSubMenu() const {
    line(); std::cout << B<<"  [1] 시료 관리\n"<<R; line('-');
    std::cout << "  "<<CY<<"[1]"<<R<<" 시료 등록   "
              <<CY<<"[2]"<<R<<" 시료 목록   "
              <<CY<<"[3]"<<R<<" 시료 검색   "
              <<RD<<"[0]"<<R<<" 뒤로\n  선택 > ";
}

static void printSampleTable(const std::vector<Sample>& list, const std::map<std::string,int>& stock) {
    if (list.empty()) { std::cout << "  (등록된 시료 없음)\n"; return; }
    std::cout << AppView::colored("") ; // force include
    std::cout << "\033[36m"
              << std::left << "  " << std::setw(8)<<"ID" << std::setw(22)<<"시료명"
              << std::setw(14)<<"생산시간(min/ea)" << std::setw(8)<<"수율" << "현재 재고\n\033[0m";
    AppView::line('-');
    for (const auto& s : list) {
        int qty = stock.count(s.id) ? stock.at(s.id) : 0;
        std::cout << "  " << std::left << std::setw(8)<<s.id << std::setw(22)<<s.name
                  << std::setw(14)<<s.avgProductionTime << std::setw(8)<<s.yield << qty << " ea\n";
    }
}

void AppView::printSampleList(const std::vector<Sample>& samples,
                               const std::map<std::string,int>& stock) const {
    std::cout << "\n  등록 시료 목록 (총 " << samples.size() << "종)\n"; line('-');
    printSampleTable(samples, stock); line('-');
}

void AppView::printSampleRegistered(const Sample& s, int initStock) const {
    printSuccess("시료 등록 완료."); line('-');
    std::cout << "  ID: " << s.id << "  이름: " << s.name
              << "  생산시간: " << s.avgProductionTime << " min/ea"
              << "  수율: " << s.yield << "  초기재고: " << initStock << " ea\n"; line('-');
}

void AppView::printSampleSearchResult(const std::vector<Sample>& samples,
                                       const std::map<std::string,int>& stock) const {
    if (samples.empty()) { std::cout << "  "<<YL<<"검색 결과가 없습니다."<<R<<"\n"; return; }
    std::cout << "\n  검색 결과 (" << samples.size() << "건)\n"; line('-');
    printSampleTable(samples, stock); line('-');
}

// ── 시료 주문 ─────────────────────────────────────────────
void AppView::printOrderForm() const {
    line(); std::cout << B<<"  [2] 시료 주문\n"<<R; line('-');
}

void AppView::printOrderConfirm(const Sample& s, const std::string& customer, int qty) const {
    std::cout << "\n  입력 내용 확인\n"; line('-');
    std::cout << "  시료     " << s.name << " (" << s.id << ")\n"
              << "  고객     " << customer << "\n"
              << "  수량     " << qty << " ea\n"; line('-');
    std::cout << "  "<<GR<<"[Y]"<<R<<" 예약 접수   "<<RD<<"[N]"<<R<<" 취소\n  선택 > ";
}

void AppView::printOrderCreated(const Order& o) const {
    printSuccess("예약 접수 완료."); line('-');
    std::cout << "  주문번호   " << o.orderId << "\n"
              << "  현재 상태  " << colored("RESERVED") << "\n"
              << "  ※ 재고 확인은 [3] 승인 메뉴에서 진행하세요.\n"; line('-');
}

// ── 주문 승인/거절 ────────────────────────────────────────
void AppView::printApprovalHeader() const {
    line(); std::cout << B<<"  [3] 주문 승인/거절\n"<<R; line('-');
}

void AppView::printReservedList(const std::vector<Order>& orders,
                                 const std::vector<Sample>& samples) const {
    std::cout << "  승인 대기 중인 예약 목록 (RESERVED)\n"; line('-');
    if (orders.empty()) { std::cout << "  (대기 중인 주문 없음)\n"; line('-'); return; }
    std::cout << CY << std::left
              << "  " << std::setw(5)<<"번호" << std::setw(22)<<"주문번호"
              << std::setw(14)<<"고객" << std::setw(14)<<"시료" << "수량\n" << R; line('-');
    int idx = 1;
    for (const auto& o : orders)
        std::cout << "  " << std::left
                  << std::setw(5) << ("["+std::to_string(idx++)+")")
                  << std::setw(22) << o.orderId << std::setw(14) << o.customerName
                  << std::setw(14) << sampleName(o.sampleId, samples) << o.quantity << " ea\n";
    line('-');
}

void AppView::printApprovalDetail(const Order& o, const Sample& s, int stock) const {
    std::cout << "\n  재고 확인 중...\n"; line('-');
    std::cout << "  시료       " << s.name << "  현재 재고 " << stock << " ea\n"
              << "  주문 수량  " << o.quantity << " ea\n";
    if (stock >= o.quantity)
        std::cout << "  " << GR << "재고 충분. 즉시 출고 대기(CONFIRMED)로 전환됩니다." << R << "\n";
    else {
        int shortage   = o.quantity - stock;
        int actualProd = static_cast<int>(std::ceil(shortage / (s.yield * 0.9)));
        double tt      = s.avgProductionTime * actualProd;
        std::cout << "  " << YL << "재고 부족.  부족분 " << shortage << " ea"
                  << " → 실생산량 " << actualProd << " ea / " << (int)tt << " min" << R << "\n";
    }
    line('-');
    std::cout << "  "<<GR<<"[Y]"<<R<<" 승인   "<<RD<<"[N]"<<R<<" 주문 거절\n  선택 > ";
}

void AppView::printApprovalResult(const Order& o, const ApprovalResult& r) const {
    if (!r.success) { printError("처리 실패."); return; }
    printSuccess("승인 완료."); line('-');
    std::cout << "  주문번호   " << o.orderId << "\n"
              << "  상태 변경  " << colored("RESERVED") << " → " << colored(statusToString(r.newStatus)) << "\n";
    if (r.newStatus == OrderStatus::PRODUCING)
        std::cout << "  부족분 " << r.shortage << " ea → 생산 라인 등록 (실생산량 "
                  << r.actualProduction << " ea / " << (int)r.totalTime << " min)\n";
    line('-');
}

void AppView::printRejected(const Order& o) const {
    printSuccess("주문 거절 완료."); line('-');
    std::cout << "  주문번호   " << o.orderId << "\n"
              << "  상태 변경  " << colored("RESERVED") << " → " << colored("REJECTED") << "\n"; line('-');
}

// ── 모니터링 ──────────────────────────────────────────────
void AppView::printMonitorHeader() const {
    line(); std::cout << B<<"  [4] 모니터링   " << nowStr() << R << "\n"; line('-');
}

void AppView::printOrderCounts(const OrderCounts& c) const {
    std::cout << "  상태별 주문 현황\n"; line('-');
    std::cout << "  " << colored("RESERVED")  << "   " << c.reserved  << "건\n"
              << "  " << colored("PRODUCING") << "  " << c.producing << "건"
              << (c.producing>0?" ← 생산라인 대기":"") << "\n"
              << "  " << colored("CONFIRMED") << "  " << c.confirmed << "건\n"
              << "  " << colored("RELEASED")  << "   " << c.released  << "건\n";
    line('-');
    std::cout << "  합계 (REJECTED 제외)   "
              << c.reserved+c.producing+c.confirmed+c.released << "건\n"; line('-');
}

void AppView::printInventory(const std::vector<InventoryInfo>& items) const {
    std::cout << "\n  재고 현황\n"; line('-');
    if (items.empty()) { std::cout << "  (등록된 시료 없음)\n"; line('-'); return; }
    int maxStock = 1;
    for (const auto& i : items) maxStock = std::max(maxStock, i.stock);
    std::cout << CY << std::left << std::setw(22)<<"  시료명" << std::setw(10)<<"재고"
              << std::setw(8)<<"상태" << "잔여율\n" << R; line('-');
    for (const auto& i : items) {
        int  bar = maxStock>0 ? i.stock*16/maxStock : 0;
        int  pct = maxStock>0 ? i.stock*100/maxStock : 0;
        std::string stColor = (i.stockStatus=="여유") ? GR : (i.stockStatus=="부족") ? YL : RD;
        std::cout << "  " << std::left << std::setw(22) << i.sampleName
                  << std::setw(10) << (std::to_string(i.stock)+" ea")
                  << std::setw(8)  << (stColor+i.stockStatus+R)
                  << "[" << GR << std::string(bar,'#') << R << std::string(16-bar,'.')
                  << "] " << pct << "%\n";
    }
    line('-');
}

// ── 생산라인 ──────────────────────────────────────────────
void AppView::printProductionHeader() const {
    line(); std::cout << B<<"  [5] 생산라인 조회   FIFO 방식\n"<<R; line('-');
}

void AppView::printProductionLine(const std::optional<ProductionJob>& current,
                                   const std::vector<ProductionJob>& waiting,
                                   const std::vector<Sample>& samples) const {
    std::cout << "  생산라인 1개 (단일 라인)   현재 상태: ";
    if (current) std::cout << GR << "RUNNING" << R << "\n";
    else         std::cout << YL << "IDLE"    << R << "\n";
    line('-');
    if (current) {
        const auto& j = *current;
        std::cout << CY<<"  현재 처리 중\n"<<R; line('-');
        std::cout << "  주문번호   " << j.orderId << "\n"
                  << "  시료       " << sampleName(j.sampleId, samples) << "\n"
                  << "  주문량     " << j.orderQty << " ea   재고 " << j.currentStock
                  << " ea → 부족 " << j.shortage << " ea\n"
                  << "  실생산량   " << j.actualProduction << " ea  (수율 "
                  << j.yield << " / " << (int)j.totalTime << " min)\n"; line('-');
    } else {
        std::cout << "  (현재 생산 중인 작업 없음)\n"; line('-');
    }
    std::cout << "  대기 중인 주문 (FIFO 순)  " << waiting.size() << "건\n";
    if (!waiting.empty()) {
        line('-');
        std::cout << CY << std::left
                  << "  "<<std::setw(5)<<"순서"<<std::setw(22)<<"주문번호"
                  <<std::setw(16)<<"시료"<<std::setw(10)<<"주문량"<<std::setw(8)<<"부족분"<<"실생산량\n"<<R;
        line('-');
        int idx = 1;
        for (const auto& j : waiting)
            std::cout << "  " << std::left << std::setw(5)<<idx++
                      << std::setw(22)<<j.orderId << std::setw(16)<<sampleName(j.sampleId,samples)
                      << std::setw(10)<<(std::to_string(j.orderQty)+" ea")
                      << std::setw(8) <<(std::to_string(j.shortage)+" ea")
                      << j.actualProduction << " ea\n";
    }
    line('-');
    if (current) std::cout << "  "<<GR<<"[C]"<<R<<" 현재 작업 생산 완료 처리   "<<RD<<"[0]"<<R<<" 뒤로\n";
    else         std::cout << "  "<<RD<<"[0]"<<R<<" 뒤로\n";
    std::cout << "  선택 > ";
}

void AppView::printProductionCompleted(const ProductionJob& job) const {
    printSuccess("생산 완료 처리."); line('-');
    std::cout << "  주문번호   " << job.orderId << "\n"
              << "  생산량     " << job.actualProduction
              << " ea (양품 " << (int)(job.actualProduction*job.yield) << " ea)\n"
              << "  주문 상태  " << colored("PRODUCING") << " → " << colored("CONFIRMED") << "\n"; line('-');
}

// ── 출고 처리 ─────────────────────────────────────────────
void AppView::printReleaseHeader() const {
    line(); std::cout << B<<"  [6] 출고 처리\n"<<R; line('-');
}

void AppView::printConfirmedList(const std::vector<Order>& orders,
                                  const std::vector<Sample>& samples) const {
    std::cout << "  출고 가능 주문 (CONFIRMED)\n"; line('-');
    if (orders.empty()) { std::cout << "  (출고 가능 주문 없음)\n"; line('-'); return; }
    std::cout << CY << std::left
              << "  "<<std::setw(5)<<"번호"<<std::setw(22)<<"주문번호"
              <<std::setw(16)<<"고객"<<std::setw(16)<<"시료"<<"수량\n"<<R; line('-');
    int idx = 1;
    for (const auto& o : orders)
        std::cout << "  " << std::left
                  << std::setw(5) << ("["+std::to_string(idx++)+")")
                  << std::setw(22) << o.orderId << std::setw(16) << o.customerName
                  << std::setw(16) << sampleName(o.sampleId, samples) << o.quantity << " ea\n";
    line('-'); std::cout << "  출고할 번호 입력 (0=취소) > ";
}

void AppView::printReleased(const Order& o) const {
    std::string t = nowStr();
    printSuccess("출고 처리 완료."); line('-');
    std::cout << "  주문번호   " << o.orderId << "\n"
              << "  출고 수량  " << o.quantity << " ea\n"
              << "  처리 일시  " << t << "\n"
              << "  상태       " << colored("CONFIRMED") << " → " << colored("RELEASED") << "\n"; line('-');
}
