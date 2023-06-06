cmake_minimum_required(VERSION 3.14)

project(hw-2)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(hw-2 ${SOURCES} include/MMU.h include/MMU_Config.h src/MMU.cpp src/MMU_Config.cpp src/MMU_Statistics.cpp include/MMU_Statistics.h src/main.cpp)

