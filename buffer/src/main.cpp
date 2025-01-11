#include "market_data.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    MarketDataProcessor processor;

    std::cout << "Starting market data processor..." << std::endl;
    processor.start();

    // Run for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "Stopping market data processor..." << std::endl;
    processor.stop();

    return 0;
}
