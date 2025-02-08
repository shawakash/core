cmake_minimum_required(VERSION 3.10)
project(Patterns)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags for optimization
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Fetch nlohmann/json
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Add executable
add_executable(strategy
    src/strategy/strategy.cpp
)

add_executable(observer
    src/utils/http.cpp
    src/utils/http.hpp
    src/config.cpp
    src/observer/observer.cpp
)

add_executable(decorator
    src/utils/http.cpp
    src/utils/http.hpp
    src/config.cpp
    src/decorator/exchange/binance.cpp
    src/decorator/exchange/bybit.cpp
    src/decorator/exchange/coinbase.cpp
    src/decorator/exchange/okx.cpp
    src/decorator/decorator.cpp
    src/decorator/interface.hpp
    src/decorator/arber.bot.cpp
    src/decorator/main.cpp
)

# Find CURL
find_package(CURL REQUIRED)
target_link_libraries(observer
    PRIVATE CURL::libcurl
    PRIVATE nlohmann_json::nlohmann_json
)

# Find CURL
find_package(CURL REQUIRED)
target_link_libraries(decorator
    PRIVATE CURL::libcurl
    PRIVATE nlohmann_json::nlohmann_json
)

# Google Test
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
