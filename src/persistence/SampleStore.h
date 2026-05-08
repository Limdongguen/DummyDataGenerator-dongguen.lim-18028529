#pragma once
#include <vector>
#include <optional>
#include <string>
#include "model/Models.h"

class SampleStore {
public:
    explicit SampleStore(const std::string& dataDir = "data");
    std::string nextId();
    void add(const Sample& sample);
    std::vector<Sample> findAll() const;
    std::optional<Sample> findById(const std::string& id) const;
    std::vector<Sample> findByNameKeyword(const std::string& keyword) const;
    bool exists(const std::string& id) const;
    int  count() const { return static_cast<int>(samples_.size()); }
    void reload() { samples_.clear(); nextSeq_ = 1; load(); }
private:
    void load(); void save() const;
    std::string path_; std::vector<Sample> samples_; int nextSeq_ = 1;
};
