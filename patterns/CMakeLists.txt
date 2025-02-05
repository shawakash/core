cmake_minimum_required(VERSION 3.10)
project(Patterns)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags for optimization
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add executable
add_executable(patterns
    src/strategy.cpp
)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
FetchContent_MakeAvailable(googletest)

# Add test executable
#add_executable(run_tests
#)

#target_link_libraries(run_tests
#    GTest::gtest_main
#    GTest::gmock_main
#)
