#define NOMINMAX
#include "generator/DataGenerator.h"
#include "writer/JsonWriter.h"
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>

static void setupConsole() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0; GetConsoleMode(hOut, &mode);
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

static void printHeader() {
    std::cout << "==============================================================\n";
    std::cout << "\033[1m\033[36m  DummyDataGenerator — 반도체 시료 더미 데이터 생성 도구\033[0m\n";
    std::cout << "==============================================================\n";
}

static void printSummary(const DummyDataSet& ds, const std::string& outDir) {
    // 상태별 주문 집계
    int res=0, pro=0, con=0, rel=0, rej=0;
    for (const auto& o : ds.orders) {
        switch (o.status) {
        case OrderStatus::RESERVED:  ++res; break;
        case OrderStatus::PRODUCING: ++pro; break;
        case OrderStatus::CONFIRMED: ++con; break;
        case OrderStatus::RELEASED:  ++rel; break;
        case OrderStatus::REJECTED:  ++rej; break;
        }
    }
    int totalStock = 0;
    for (const auto& [k,v] : ds.inventory) totalStock += v;

    std::cout << "\n  \033[32m생성 완료.\033[0m  출력 경로: " << outDir << "\n";
    std::cout << "--------------------------------------------------------------\n";
    std::cout << "  시료     " << ds.samples.size() << "종\n";
    std::cout << "  주문     " << ds.orders.size() << "건"
              << "  (RESERVED:" << res << " CONFIRMED:" << con
              << " PRODUCING:" << pro << " RELEASED:" << rel << ")\n";
    std::cout << "  총 재고  " << totalStock << " ea\n";
    std::cout << "  생산 큐  " << ds.productionJobs.size() << "건\n";
    std::cout << "--------------------------------------------------------------\n";
    std::cout << "  \033[36msamples.json\033[0m    \033[36morders.json\033[0m"
              << "    \033[36minventory.json\033[0m    \033[36mproduction.json\033[0m\n";
    std::cout << "--------------------------------------------------------------\n";
}

int main(int argc, char* argv[]) {
    setupConsole();

    // CLI 모드: DummyDataGenerator.exe <출력경로> <시료수> <주문수> [시드]
    if (argc >= 4) {
        std::string outDir = argv[1];
        int sc = std::stoi(argv[2]);
        int oc = std::stoi(argv[3]);
        unsigned int seed = (argc >= 5) ? static_cast<unsigned int>(std::stoi(argv[4])) : 0;

        DataGenerator gen(seed);
        auto ds = gen.generate(sc, oc);
        JsonWriter writer(outDir);
        writer.write(ds);
        printSummary(ds, outDir);
        return 0;
    }

    // 대화형 메뉴
    printHeader();
    int ch;
    do {
        std::cout << "\n  \033[36m[1]\033[0m 표준 더미 데이터 생성 (시료 5종, 주문 10건)\n";
        std::cout << "  \033[36m[2]\033[0m 사용자 정의 생성\n";
        std::cout << "  \033[36m[3]\033[0m 대용량 테스트 데이터 생성 (시료 10종, 주문 50건)\n";
        std::cout << "  \033[31m[0]\033[0m 종료\n";
        std::cout << "  선택 > ";

        if (!(std::cin >> ch)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            ch = -1; continue;
        }

        if (ch == 0) { std::cout << "  종료합니다.\n"; break; }

        std::string outDir = "data";
        int sc = 5, oc = 10;
        unsigned int seed = 0;

        if (ch == 1) {
            // 표준
        } else if (ch == 2) {
            std::cout << "  출력 경로: "; std::cin >> outDir;
            std::cout << "  시료 수  : "; std::cin >> sc;
            std::cout << "  주문 수  : "; std::cin >> oc;
            std::cout << "  시드(0=랜덤): "; std::cin >> seed;
        } else if (ch == 3) {
            sc = 10; oc = 50;
        } else {
            std::cout << "  잘못된 입력\n"; continue;
        }

        DataGenerator gen(seed);
        auto ds = gen.generate(sc, oc);
        JsonWriter writer(outDir);
        writer.write(ds);
        printSummary(ds, outDir);

    } while (true);

    return 0;
}
