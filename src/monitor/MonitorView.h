#pragma once
#include "model/Models.h"

class MonitorView {
public:
    // 전체 대시보드 (auto-refresh 화면)
    void printDashboard(const DataSnapshot& snap, int countdown) const;

    // 메인 메뉴
    void printMenu(const std::string& dataDir) const;

    // 상세 화면
    void printOrderList(const DataSnapshot& snap) const;
    void printInventoryDetail(const DataSnapshot& snap) const;
    void printProductionDetail(const DataSnapshot& snap) const;

    void printError(const std::string& msg) const;

    static void clearScreen();
    static void line(char c = '=', int w = 62);
    static std::string colored(const std::string& status);
    static std::string stockBar(int val, int max, int width = 16);
};
