#pragma once
#include <string>
#include "model/Models.h"

// 반도체 시료 생산주문관리용 더미 데이터 생성기
class DataGenerator {
public:
    explicit DataGenerator(unsigned int seed = 0); // seed=0 이면 time 기반

    // 전체 더미 데이터 세트 생성
    DummyDataSet generate(int sampleCount = 5, int orderCount = 10);

private:
    Sample      makeSample(int seq);
    Order       makeOrder(int seq, const std::vector<Sample>& samples);
    void        buildInventoryAndJobs(DummyDataSet& ds);

    unsigned int nextRand();

    unsigned int seed_;
    int          sampleSeq_ = 1;
    int          orderSeq_  = 1;

    // 사전 정의 템플릿
    static const std::vector<std::pair<std::string,double>> SAMPLE_TEMPLATES; // {name, yield}
    static const std::vector<double>  AVG_TIMES;
    static const std::vector<std::string> CUSTOMERS;
    static const std::vector<OrderStatus> STATUS_POOL;
};
