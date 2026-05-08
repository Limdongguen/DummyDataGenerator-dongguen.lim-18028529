#define NOMINMAX
#include "monitor/MonitorApp.h"
#include "generator/DataGenerator.h"
#include "writer/JsonWriter.h"
#include <iostream>
#include <string>
#include <limits>
#include <ctime>
#include <Windows.h>
#include <io.h>
#include <conio.h>

static void clearIn() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

MonitorApp::MonitorApp(const std::string& dataDir, int refreshSec)
    : service_(dataDir), refreshSec_(refreshSec) {}

void MonitorApp::run() {
    int ch;
    do {
        appView_.printMainMenu(service_.getSystemStats());
        if (!(std::cin >> ch)) { clearIn(); ch = -1; continue; }
        std::cout << "\n";
        switch (ch) {
        case 1: sampleManagement();  break;
        case 2: placeOrder();        break;
        case 3: orderApproval();     break;
        case 4: runDashboard();      break;
        case 5: productionLine();    break;
        case 6: releaseProcessing(); break;
        case 7: generateDummyData(); break;
        case 0: std::cout << "  시스템을 종료합니다.\n"; break;
        default: appView_.printError("잘못된 입력입니다."); break;
        }
        if (ch != 0 && ch != -1) std::cout << "\n";
    } while (ch != 0);
}

// ── [1] 시료 관리 ──────────────────────────────────────────
void MonitorApp::sampleManagement() {
    int ch;
    do {
        appView_.printSampleSubMenu();
        if (!(std::cin >> ch)) { clearIn(); ch = -1; continue; }
        std::cout << "\n";
        if (ch == 1) {
            std::string name; double avgTime, yield; int initStock;
            std::cout << "  시료명: ";           std::cin >> name;
            std::cout << "  평균생산시간(min/ea): "; std::cin >> avgTime;
            std::cout << "  수율(0~1): ";         std::cin >> yield;
            std::cout << "  초기 재고(ea): ";      std::cin >> initStock;
            if (avgTime <= 0 || yield <= 0 || yield > 1) { appView_.printError("잘못된 입력 값"); continue; }
            auto s = service_.registerSample(name, avgTime, yield, initStock);
            appView_.printSampleRegistered(s, initStock);
        } else if (ch == 2) {
            appView_.printSampleList(service_.getAllSamples(), service_.getAllStock());
        } else if (ch == 3) {
            std::string kw;
            std::cout << "  검색어: "; std::cin >> kw;
            appView_.printSampleSearchResult(service_.searchSamples(kw), service_.getAllStock());
        }
    } while (ch != 0);
}

// ── [2] 시료 주문 ──────────────────────────────────────────
void MonitorApp::placeOrder() {
    appView_.printOrderForm();
    std::string sampleId, customer; int qty;
    std::cout << "  시료 ID > "; std::cin >> sampleId;
    auto sOpt = service_.getSampleById(sampleId);
    if (!sOpt) { appView_.printError("존재하지 않는 시료 ID"); return; }
    std::cout << "  고객명  > "; std::cin >> customer;
    std::cout << "  수량    > "; std::cin >> qty;
    if (qty <= 0) { appView_.printError("수량은 1 이상이어야 합니다."); return; }
    appView_.printOrderConfirm(*sOpt, customer, qty);
    char c; std::cin >> c;
    if (c != 'Y' && c != 'y') { std::cout << "  취소되었습니다.\n"; return; }
    appView_.printOrderCreated(service_.createOrder(sampleId, customer, qty));
}

// ── [3] 주문 승인/거절 ─────────────────────────────────────
void MonitorApp::orderApproval() {
    appView_.printApprovalHeader();
    auto reserved = service_.getReservedOrders();
    auto samples  = service_.getAllSamples();
    appView_.printReservedList(reserved, samples);
    if (reserved.empty()) return;

    int idx;
    std::cout << "  승인할 번호 > ";
    if (!(std::cin >> idx) || idx < 1 || idx > (int)reserved.size()) {
        clearIn(); appView_.printError("잘못된 번호"); return;
    }
    const auto& o = reserved[idx - 1];
    auto sOpt = service_.getSampleById(o.sampleId);
    if (!sOpt) { appView_.printError("시료 정보 없음"); return; }
    appView_.printApprovalDetail(o, *sOpt, service_.getSampleStock(o.sampleId));
    char c; std::cin >> c;
    if (c == 'Y' || c == 'y')
        appView_.printApprovalResult(o, service_.approveOrder(o.orderId));
    else {
        service_.rejectOrder(o.orderId);
        appView_.printRejected(o);
    }
}

static DataSnapshot buildSnap(AppService& svc) {
    DataSnapshot snap;
    snap.samples  = svc.getAllSamples();
    snap.orders   = svc.getAllOrders();
    snap.inventory= svc.getAllStock();
    if (svc.getCurrentJob()) snap.productionJobs.push_back(*svc.getCurrentJob());
    for (const auto& j : svc.getWaitingJobs()) snap.productionJobs.push_back(j);
    snap.valid = true;
    std::time_t t = std::time(nullptr);
    struct tm tm_buf; localtime_s(&tm_buf, &t);
    char buf[32]; std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
    snap.loadTime = buf;
    return snap;
}

// ── [4] 실시간 모니터링 대시보드 ──────────────────────────
void MonitorApp::runDashboard() {
    // stdin이 파일 리디렉션이면 단순 1회 출력 후 반환 (자동 갱신 불가)
    bool isTTY = (_isatty(_fileno(stdin)) != 0);
    if (!isTTY) {
        MonitorView::clearScreen();
        monView_.printDashboard(buildSnap(service_), 0);
        return;
    }

    bool running = true;
    while (running) {
        MonitorView::clearScreen();
        monView_.printDashboard(buildSnap(service_), refreshSec_);

        for (int i = 0; i < refreshSec_ * 10 && running; ++i) {
            Sleep(100);
            if (_kbhit()) {
                char key = static_cast<char>(_getch());
                if (key == 'q' || key == 'Q') { running = false; break; }
                if (key == 'r' || key == 'R') break;
                if (key == 'm' || key == 'M') { running = false; break; }
            }
        }
    }
    MonitorView::clearScreen();
}

// ── [5] 생산라인 ───────────────────────────────────────────
void MonitorApp::productionLine() {
    appView_.printProductionHeader();
    auto current = service_.getCurrentJob();
    auto samples = service_.getAllSamples();
    appView_.printProductionLine(current, service_.getWaitingJobs(), samples);
    if (!current) return;
    std::string input; std::cin >> input;
    if (input == "C" || input == "c") {
        auto job = *current;
        if (service_.completeCurrentJob()) appView_.printProductionCompleted(job);
        else appView_.printError("생산 완료 처리 실패.");
    }
}

// ── [6] 출고 처리 ──────────────────────────────────────────
void MonitorApp::releaseProcessing() {
    appView_.printReleaseHeader();
    auto confirmed = service_.getConfirmedOrders();
    auto samples   = service_.getAllSamples();
    appView_.printConfirmedList(confirmed, samples);
    if (confirmed.empty()) return;
    int idx;
    if (!(std::cin >> idx) || idx == 0) { clearIn(); std::cout << "  취소.\n"; return; }
    if (idx < 1 || idx > (int)confirmed.size()) { appView_.printError("잘못된 번호"); return; }
    const auto& o = confirmed[idx - 1];
    if (service_.release(o.orderId)) appView_.printReleased(o);
    else appView_.printError("출고 처리 실패.");
}

// ── [7] 더미 데이터 생성 ───────────────────────────────────
void MonitorApp::generateDummyData() {
    AppView::line();
    std::cout << "\033[1m  [7] 더미 데이터 생성\033[0m\n";
    AppView::line('-');
    std::cout << "  기존 데이터를 \033[33m모두 초기화\033[0m하고 더미 데이터로 교체합니다.\n\n";
    std::cout << "  \033[36m[1]\033[0m 표준  (시료 5종, 주문 10건)\n";
    std::cout << "  \033[36m[2]\033[0m 대용량 (시료 10종, 주문 50건)\n";
    std::cout << "  \033[36m[3]\033[0m 사용자 정의\n";
    std::cout << "  \033[31m[0]\033[0m 취소\n";
    std::cout << "  선택 > ";

    int ch; std::cin >> ch;
    if (ch == 0 || !(ch >= 1 && ch <= 3)) { std::cout << "  취소.\n"; return; }

    int sc = 5, oc = 10;
    unsigned int seed = 0;
    if (ch == 2) { sc = 10; oc = 50; }
    else if (ch == 3) {
        std::cout << "  시료 수(최대 10): "; std::cin >> sc;
        std::cout << "  주문 수: ";          std::cin >> oc;
        std::cout << "  시드(0=랜덤): ";     std::cin >> seed;
    }

    std::cout << "\n  생성 중...\n";
    DataGenerator gen(seed);
    auto ds = gen.generate(sc, oc);

    JsonWriter writer(service_.dataDir());
    writer.write(ds);
    service_.reloadAllStores();

    int res=0, pro=0, con=0, rel=0;
    for (const auto& o : ds.orders) {
        if (o.status==OrderStatus::RESERVED)  ++res;
        if (o.status==OrderStatus::PRODUCING) ++pro;
        if (o.status==OrderStatus::CONFIRMED) ++con;
        if (o.status==OrderStatus::RELEASED)  ++rel;
    }
    int totalStock = 0;
    for (const auto& [k,v] : ds.inventory) totalStock += v;

    std::cout << "\033[32m\n  더미 데이터 생성 완료.\033[0m\n";
    AppView::line('-');
    std::cout << "  시료   " << ds.samples.size() << "종   총 재고 " << totalStock << " ea\n";
    std::cout << "  주문   " << ds.orders.size() << "건"
              << "  (RESERVED:" << res << " CONFIRMED:" << con
              << " PRODUCING:" << pro << " RELEASED:" << rel << ")\n";
    std::cout << "  생산큐 " << ds.productionJobs.size() << "건\n";
    AppView::line('-');
}
