#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>
#include "ring_buffer.hpp"

class RingBufferTest : public ::testing::Test {
protected:
    static constexpr size_t BUFFER_SIZE = 16;
    RingBuffer<int, BUFFER_SIZE> buffer;
};

// Basic functionality tests
TEST_F(RingBufferTest, PushAndPop) {
    int value;
    EXPECT_TRUE(buffer.push(42));
    EXPECT_TRUE(buffer.pop(value));
    EXPECT_EQ(value, 42);
}

TEST_F(RingBufferTest, EmptyBuffer) {
    int value;
    EXPECT_FALSE(buffer.pop(value));
}

TEST_F(RingBufferTest, FullBuffer) {
    for (int i = 0; i < BUFFER_SIZE - 1; ++i) {
        EXPECT_TRUE(buffer.push(i));
    }
    EXPECT_FALSE(buffer.push(100));  // Buffer should be full
}

TEST_F(RingBufferTest, SizeTracking) {
    EXPECT_EQ(buffer.size(), 0);
    buffer.push(1);
    EXPECT_EQ(buffer.size(), 1);
    buffer.push(2);
    EXPECT_EQ(buffer.size(), 2);

    int value;
    buffer.pop(value);
    EXPECT_EQ(buffer.size(), 1);
}

// Multi-threaded tests
TEST_F(RingBufferTest, MultiThreadedPushPop) {
    static constexpr int NUM_OPERATIONS = 10000;
    std::atomic<int> producer_count{0};
    std::atomic<int> consumer_count{0};

    // Producer thread
    auto producer = [&]() {
        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            while (!buffer.push(i)) {
                std::this_thread::yield();
            }
            producer_count++;
        }
    };

    // Consumer thread
    auto consumer = [&]() {
        int value;
        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            while (!buffer.pop(value)) {
                std::this_thread::yield();
            }
            consumer_count++;
            EXPECT_EQ(value, i);
        }
    };

    std::thread prod_thread(producer);
    std::thread cons_thread(consumer);

    prod_thread.join();
    cons_thread.join();

    EXPECT_EQ(producer_count.load(), NUM_OPERATIONS);
    EXPECT_EQ(consumer_count.load(), NUM_OPERATIONS);
}

// Performance test
TEST_F(RingBufferTest, LatencyMeasurement) {
    static constexpr int NUM_SAMPLES = 10000;
    std::vector<double> latencies;
    latencies.reserve(NUM_SAMPLES);

    for (int i = 0; i < NUM_SAMPLES; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        buffer.push(i);
        int value;
        buffer.pop(value);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        latencies.push_back(duration.count());
    }

    // Calculate statistics
    double sum = 0;
    for (double latency : latencies) {
        sum += latency;
    }
    double avg = sum / NUM_SAMPLES;

    std::sort(latencies.begin(), latencies.end());
    double p99 = latencies[static_cast<size_t>(NUM_SAMPLES * 0.99)];

    std::cout << "Average latency: " << avg << " ns\n";
    std::cout << "P99 latency: " << p99 << " ns\n";

    // Assert reasonable performance
    EXPECT_LT(p99, 1000);  // P99 should be under 1 microsecond
}
