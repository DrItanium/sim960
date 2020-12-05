cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(instruction_test0 test0.cc)

target_link_libraries(instruction_test0 PUBLIC core960)
target_include_directories(instruction_test0 PUBLIC "${PROJECT_SRC_DIR}")
