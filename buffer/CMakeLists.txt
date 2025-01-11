cmake_minimum_required(VERSION 3.10)
project(MarketDataProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags for optimization
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add executable
add_executable(market_data_processor
    src/main.cpp
    src/market_data.cpp
)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
FetchContent_MakeAvailable(googletest)

# Add test executable
add_executable(run_tests
    tests/ring_buffer_tests.cpp
    tests/market_data_tests.cpp
    src/market_data.cpp
)

target_link_libraries(run_tests
    GTest::gtest_main
    GTest::gmock_main
)

# Add dependencies for httplib and nlohmann/json
include(FetchContent)

# Fetch httplib
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.13.1 # Replace with the desired version
)
FetchContent_MakeAvailable(httplib)

# Fetch nlohmann/json
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz # Replace with the desired version
)
FetchContent_MakeAvailable(nlohmann_json)

# Link httplib and nlohmann/json to your executables
target_link_libraries(market_data_processor
    httplib
    nlohmann_json::nlohmann_json
)

target_link_libraries(run_tests
    httplib
    nlohmann_json::nlohmann_json
)
