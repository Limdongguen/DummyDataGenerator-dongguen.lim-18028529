#include "persistence/SampleStore.h"
#include "persistence/JsonHelper.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

SampleStore::SampleStore(const std::string& dataDir) {
    json::ensureDataDir(dataDir);
    path_ = dataDir + "/samples.json";
    load();
}

void SampleStore::load() {
    std::string content = json::readFile(path_);
    if (content.empty()) return;
    nextSeq_ = json::extractInt(content, "nextSeq");
    if (nextSeq_ < 1) nextSeq_ = 1;
    for (const auto& obj : json::extractObjects(content, "samples")) {
        Sample s;
        s.id                = json::extractStr(obj, "id");
        s.name              = json::extractStr(obj, "name");
        s.avgProductionTime = json::extractDbl(obj, "avgProductionTime");
        s.yield             = json::extractDbl(obj, "yield");
        if (!s.id.empty()) samples_.push_back(s);
    }
}

void SampleStore::save() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{\n  \"nextSeq\": " << nextSeq_ << ",\n  \"samples\": [\n";
    for (size_t i = 0; i < samples_.size(); ++i) {
        const auto& s = samples_[i];
        oss << "    {\n"
            << "      \"id\": \""   << s.id   << "\",\n"
            << "      \"name\": \"" << s.name << "\",\n"
            << "      \"avgProductionTime\": " << s.avgProductionTime << ",\n"
            << "      \"yield\": "  << s.yield << "\n"
            << "    }" << (i + 1 < samples_.size() ? "," : "") << "\n";
    }
    oss << "  ]\n}\n";
    json::writeFile(path_, oss.str());
}

std::string SampleStore::nextId() {
    std::ostringstream oss;
    oss << "S-" << std::setw(3) << std::setfill('0') << nextSeq_++;
    return oss.str();
}

void SampleStore::add(const Sample& sample) {
    samples_.push_back(sample);
    save();
}

std::vector<Sample> SampleStore::findAll() const { return samples_; }

std::optional<Sample> SampleStore::findById(const std::string& id) const {
    for (const auto& s : samples_) if (s.id == id) return s;
    return std::nullopt;
}

std::vector<Sample> SampleStore::findByNameKeyword(const std::string& keyword) const {
    std::vector<Sample> result;
    for (const auto& s : samples_)
        if (s.name.find(keyword) != std::string::npos) result.push_back(s);
    return result;
}

bool SampleStore::exists(const std::string& id) const {
    return findById(id).has_value();
}
