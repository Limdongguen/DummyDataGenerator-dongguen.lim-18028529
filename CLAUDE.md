# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 프로젝트 개요

**DummyDataGenerator-dongguen.lim-18028529** — 반도체 시료 생산주문관리 시스템 테스트용 더미 데이터 생성 도구.  
DataPersistence / DataMonitor POC와 호환되는 JSON 파일 4종을 생성한다.

## 빌드 및 실행

### CMake
```powershell
cmake -B build && cmake --build build

# CLI 모드
.\build\dummy-data-generator.exe <출력경로> <시료수> <주문수> [시드]
.\build\dummy-data-generator.exe data 5 10 42

# 대화형 모드
.\build\dummy-data-generator.exe
```

### Visual Studio
- 프로젝트: `DummyDataGenerator_POC\DummyDataGenerator_POC.vcxproj` (x64, C++20)

### 테스트 실행
```powershell
powershell -ExecutionPolicy Bypass -File test\run_test.ps1
```

## 아키텍처

```
src/
├── main.cpp               ← CLI 인수 파싱 + 대화형 메뉴
├── model/Models.h         ← 도메인 모델 (Sample, Order, ProductionJob, DummyDataSet)
├── generator/
│   └── DataGenerator.h/.cpp  ← 더미 데이터 생성 (LCG 난수, 시드 재현성)
└── writer/
    └── JsonWriter.h/.cpp     ← DummyDataSet → 4개 JSON 파일 출력
```

## 생성 파일 형식 (DataPersistence 호환)

| 파일 | 내용 |
|------|------|
| `samples.json` | 시료 목록 + nextSeq |
| `orders.json` | 주문 목록 + 상태 + nextSeq |
| `inventory.json` | 시료별 재고 |
| `production.json` | PRODUCING 주문의 생산 큐 |

## 핵심 특징

- **시드 고정**: `seed` 인수를 주면 동일한 데이터가 재현됨 (테스트 재현성)
- **시료 템플릿**: 실제 반도체 시료명 10종 사전 정의 (실리콘 웨이퍼, GaN 에피택셜 등)
- **상태 분포**: RESERVED·CONFIRMED·PRODUCING·RELEASED 혼합 생성

## 테스트

`test/run_test.ps1` — 4개 시나리오, 27개 항목 자동 검증:
1. 표준 생성 (5종, 10건, seed 42)
2. 대용량 생성 (10종, 50건, seed 99)
3. 재현성 (같은 seed → 동일 출력)
4. JSON 파싱 유효성 (ConvertFrom-Json)
