cmake_minimum_required(VERSION 3.16)

project(embedded_smart_greenhouse_cluster
    VERSION 1.0.0
    DESCRIPTION "C++ embedded smart greenhouse vegetable cluster simulator"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(greenhouse_cluster
    src/main.cpp
    src/GreenhouseNode.cpp
    src/ClusterController.cpp
)

target_include_directories(greenhouse_cluster PRIVATE include)
