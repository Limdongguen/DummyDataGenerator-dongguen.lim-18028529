cmake_minimum_required(VERSION 3.20)
project(dummy-data-generator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/persistence/JsonHelper.cpp
    src/persistence/SampleStore.cpp
    src/persistence/OrderStore.cpp
    src/persistence/InventoryStore.cpp
    src/persistence/ProductionStore.cpp
    src/service/AppService.cpp
    src/view/AppView.cpp
    src/reader/DataReader.cpp
    src/generator/DataGenerator.cpp
    src/writer/JsonWriter.cpp
    src/monitor/MonitorView.cpp
    src/monitor/MonitorApp.cpp
)

add_executable(dummy-data-generator ${SOURCES})
target_include_directories(dummy-data-generator PRIVATE src)
