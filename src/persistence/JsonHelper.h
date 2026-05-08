#pragma once
#include <string>
#include <vector>

// JSON 파일 읽기/쓰기 + 단순 파싱 유틸리티
namespace json {

std::string  readFile(const std::string& path);
void         writeFile(const std::string& path, const std::string& content);

// 단일 JSON 객체에서 값 추출
std::string  extractStr(const std::string& obj, const std::string& key);
double       extractDbl(const std::string& obj, const std::string& key);
int          extractInt(const std::string& obj, const std::string& key);

// 최상위 배열에서 { ... } 객체들 분리
std::vector<std::string> extractObjects(const std::string& json, const std::string& arrayKey);

// data/ 디렉토리가 없으면 생성
void ensureDataDir(const std::string& dir = "data");

} // namespace json
