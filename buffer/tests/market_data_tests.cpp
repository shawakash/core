#include <chrono>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "market_data.hpp"
#include <string>
#include <thread>

class MarketDataTest : public ::testing::Test {
protected:
    MarketDataProcessor processor;
};

// Test MarketUpdate structure
TEST_F(MarketDataTest, MarketUpdateConstruction) {
    std::string symbol("BTCUSDC");
    const char side = 'B';
    MarketUpdate update(1234567890, 50000.0, 1.5, symbol, side);

    EXPECT_EQ(update.timestamp, 1234567890);
    EXPECT_DOUBLE_EQ(update.price, 50000.0);
    EXPECT_DOUBLE_EQ(update.quantity, 1.5);
    EXPECT_STREQ(update.symbol, "BTCUSDC");
    EXPECT_EQ(update.side, 'B');
}

// Test market data processing
TEST_F(MarketDataTest, ProcessorStartStop) {
    processor.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    processor.stop();
    // Basic test to ensure no crashes
    SUCCEED();
}

// Mock MarketDataProcessor for testing
class MockMarketDataProcessor : public MarketDataProcessor {
public:
    MOCK_METHOD(void, processUpdate, (const MarketUpdate&), ());
};

// Stress test
TEST_F(MarketDataTest, StressTest) {
    processor.start();

    // Generate high load of market updates
    const int NUM_UPDATES = 100000;
    std::vector<std::thread> threads;

    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < NUM_UPDATES; ++j) {
                std::string symbol("BTCUSDC");
                const char side = 'B';
                MarketUpdate update(j, 50000.0 + j, 1.0, symbol, side);
                // Simulate high-frequency updates
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    processor.stop();
}
