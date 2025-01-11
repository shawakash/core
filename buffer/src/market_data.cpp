#include "market_data.hpp"
#include <atomic>
#include <cstdint>
#include <thread>
#include <chrono>
#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>

MarketDataProcessor::MarketDataProcessor() = default;

void MarketDataProcessor::start() {
    running_ = true;

    std::thread producer(&MarketDataProcessor::producerThread, this);
    std::thread consumer(&MarketDataProcessor::consumerThread, this);

    producer.detach();
    consumer.detach();
}

void MarketDataProcessor::stop() {
    running_ = false;
}

void MarketDataProcessor::producerThread() {
    while (running_) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

        std::string symbol("BTCUSDC");
        const char side = 'B';
        MarketUpdate update(timestamp, 100.0, 10.0, symbol, side);

        while (!market_data_buffer_.push(update)) {
            std::this_thread::yield();
        }

        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
}

void MarketDataProcessor::consumerThread() {
    MarketUpdate update;
    std::atomic<uint64_t> count{0};
    while (running_) {
        if (market_data_buffer_.pop(update)) {
            // Process the market update
            count.store(count.load(std::memory_order_acquire) + 1, std::memory_order_release);
            std::cout << "Processed " << count.load(std::memory_order_relaxed) << " update for " << update.symbol
                      << " Price: " << update.price
                      << " Quantity: " << update.quantity << std::endl;
        }
        std::this_thread::yield();
    }
}

void MarketDataProcessor::fetchData() {
    httplib::Client client("https://api.binance.com");
    auto res = client.Get("/api/v3/ticker/price?symbol=BTCUSDT");

    if (res && res->status == 200) {
        auto json = nlohmann::json::parse(res->body);
        double price = std::stod(json["price"].get<std::string>());

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

        std::string symbol("BTCUSDT");
        const char side = 'B';
        MarketUpdate update(timestamp, price, 10.0, symbol, side);

        while (!market_data_buffer_.push(update)) {
            std::this_thread::yield();
        }

        std::cout << "Pushed market update for " << symbol
                    << " Price: " << price
                    << " Timestamp: " << timestamp << std::endl;

    } else {
        std::cerr << "Failed to fetch data from Binance. Status: "
                    << (res ? std::to_string(res->status) : "No response") << std::endl;
    }
}
