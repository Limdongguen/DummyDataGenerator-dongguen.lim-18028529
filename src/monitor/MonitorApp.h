#pragma once
#include <string>
#include "service/AppService.h"
#include "view/AppView.h"
#include "monitor/MonitorView.h"

// PDF 6개 메뉴 + [4] 실시간 모니터링 대시보드 + [7] 더미 데이터 생성
class MonitorApp {
public:
    explicit MonitorApp(const std::string& dataDir = "data", int refreshSec = 3);
    void run();
private:
    void sampleManagement();          // [1]
    void placeOrder();                // [2]
    void orderApproval();             // [3]
    void runDashboard();              // [4] 실시간 자동 갱신
    void productionLine();            // [5]
    void releaseProcessing();         // [6]
    void generateDummyData();         // [7] 더미 데이터 생성

    AppService  service_;
    AppView     appView_;
    MonitorView monView_;
    int         refreshSec_;
};
