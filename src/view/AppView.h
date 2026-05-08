#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>
#include "model/Models.h"

// 모든 화면 출력을 하나의 클래스로 통합 (std::cout 전용, Service 직접 호출 금지)
class AppView {
public:
    // ── 공통 ──────────────────────────────────────────────
    void printError(const std::string& msg) const;
    void printSuccess(const std::string& msg) const;
    static void line(char c = '=', int w = 62);
    static std::string colored(const std::string& status);

    // ── 메인 메뉴 ─────────────────────────────────────────
    void printMainMenu(const SystemStats& stats) const;

    // ── 시료 관리 ─────────────────────────────────────────
    void printSampleSubMenu() const;
    void printSampleList(const std::vector<Sample>& samples,
                         const std::map<std::string,int>& stock) const;
    void printSampleRegistered(const Sample& s, int initStock) const;
    void printSampleSearchResult(const std::vector<Sample>& samples,
                                 const std::map<std::string,int>& stock) const;

    // ── 시료 주문 ─────────────────────────────────────────
    void printOrderForm() const;
    void printOrderConfirm(const Sample& s, const std::string& customer, int qty) const;
    void printOrderCreated(const Order& o) const;

    // ── 주문 승인/거절 ────────────────────────────────────
    void printApprovalHeader() const;
    void printReservedList(const std::vector<Order>& orders,
                           const std::vector<Sample>& samples) const;
    void printApprovalDetail(const Order& o, const Sample& s, int stock) const;
    void printApprovalResult(const Order& o, const ApprovalResult& r) const;
    void printRejected(const Order& o) const;

    // ── 모니터링 ──────────────────────────────────────────
    void printMonitorHeader() const;
    void printOrderCounts(const OrderCounts& counts) const;
    void printInventory(const std::vector<InventoryInfo>& items) const;

    // ── 생산라인 ──────────────────────────────────────────
    void printProductionHeader() const;
    void printProductionLine(const std::optional<ProductionJob>& current,
                              const std::vector<ProductionJob>& waiting,
                              const std::vector<Sample>& samples) const;
    void printProductionCompleted(const ProductionJob& job) const;

    // ── 출고 처리 ─────────────────────────────────────────
    void printReleaseHeader() const;
    void printConfirmedList(const std::vector<Order>& orders,
                             const std::vector<Sample>& samples) const;
    void printReleased(const Order& o) const;
};
