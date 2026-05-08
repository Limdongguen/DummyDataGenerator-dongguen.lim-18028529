#pragma once
#include <string>
#include "model/Models.h"

// JSON 파일에서 전체 데이터를 읽기 전용으로 로드
class DataReader {
public:
    explicit DataReader(const std::string& dataDir = "data");
    DataSnapshot load() const;   // 매 호출마다 파일을 새로 읽어 스냅샷 반환
    const std::string& dataDir() const { return dataDir_; }
private:
    std::string dataDir_;
};
