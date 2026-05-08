#pragma once
#include <deque>
#include <vector>
#include <optional>
#include <string>
#include "model/Models.h"

class ProductionStore {
public:
    explicit ProductionStore(const std::string& dataDir = "data");
    void enqueue(const ProductionJob& job);
    bool hasWork() const;
    std::optional<ProductionJob> currentJob() const;
    std::vector<ProductionJob>   waitingJobs() const;
    ProductionJob dequeue();
    int  totalCount() const;
    void reload() { jobs_.clear(); load(); }
private:
    void load(); void save() const;
    std::string path_; std::deque<ProductionJob> jobs_;
};
