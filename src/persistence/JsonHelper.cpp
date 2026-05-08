#include "persistence/JsonHelper.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>

namespace json {

std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream f(path, std::ios::out | std::ios::trunc);
    if (!f.is_open()) throw std::runtime_error("파일 쓰기 실패: " + path);
    f << content;
}

void ensureDataDir(const std::string& dir) {
    std::filesystem::create_directories(dir);
}

std::string extractStr(const std::string& obj, const std::string& key) {
    std::string search = "\"" + key + "\": \"";
    auto pos = obj.find(search);
    if (pos == std::string::npos) {
        search = "\"" + key + "\":\"";  // 공백 없는 버전도 시도
        pos = obj.find(search);
        if (pos == std::string::npos) return "";
    }
    pos += search.size();
    auto end = obj.find('"', pos);
    return end == std::string::npos ? "" : obj.substr(pos, end - pos);
}

static size_t skipWhitespace(const std::string& s, size_t pos) {
    while (pos < s.size() && (s[pos]==' '||s[pos]=='\n'||s[pos]=='\r'||s[pos]=='\t')) ++pos;
    return pos;
}

static std::string extractScalar(const std::string& obj, const std::string& key) {
    std::string search = "\"" + key + "\":";
    auto pos = obj.find(search);
    if (pos == std::string::npos) return "";
    pos = skipWhitespace(obj, pos + search.size());
    size_t end = pos;
    while (end < obj.size() && obj[end] != ',' && obj[end] != '}' && obj[end] != '\n') ++end;
    std::string val = obj.substr(pos, end - pos);
    // trim trailing whitespace
    while (!val.empty() && (val.back()==' '||val.back()=='\r'||val.back()=='\n')) val.pop_back();
    return val;
}

double extractDbl(const std::string& obj, const std::string& key) {
    auto s = extractScalar(obj, key);
    if (s.empty()) return 0.0;
    try { return std::stod(s); } catch (...) { return 0.0; }
}

int extractInt(const std::string& obj, const std::string& key) {
    auto s = extractScalar(obj, key);
    if (s.empty()) return 0;
    try { return std::stoi(s); } catch (...) { return 0; }
}

std::vector<std::string> extractObjects(const std::string& json, const std::string& arrayKey) {
    // 배열 시작 위치 탐색: "key": [
    std::string search = "\"" + arrayKey + "\": [";
    auto pos = json.find(search);
    if (pos == std::string::npos) {
        search = "\"" + arrayKey + "\":[";
        pos = json.find(search);
        if (pos == std::string::npos) return {};
    }
    pos += search.size();

    // 배열 끝 ] 찾기
    int depth = 0;
    size_t end = pos;
    for (size_t i = pos; i < json.size(); ++i) {
        if (json[i] == '[') ++depth;
        else if (json[i] == ']') {
            if (depth == 0) { end = i; break; }
            --depth;
        }
    }
    std::string arr = json.substr(pos, end - pos);

    // { ... } 객체 분리
    std::vector<std::string> result;
    int braceDepth = 0;
    size_t objStart = 0;
    bool inObj = false;
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] == '{') {
            if (!inObj) { objStart = i; inObj = true; }
            ++braceDepth;
        } else if (arr[i] == '}') {
            --braceDepth;
            if (braceDepth == 0 && inObj) {
                result.push_back(arr.substr(objStart, i - objStart + 1));
                inObj = false;
            }
        }
    }
    return result;
}

} // namespace json
