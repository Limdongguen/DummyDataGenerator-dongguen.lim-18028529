cmake_minimum_required(VERSION 3.20)
project(dummy-data-generator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/generator/DataGenerator.cpp
    src/writer/JsonWriter.cpp
)

add_executable(dummy-data-generator ${SOURCES})
target_include_directories(dummy-data-generator PRIVATE src)
