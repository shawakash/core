#pragma once
#include <cstdint>
#include <string>
#include <sys/socket.h>
#include "ring_buffer.hpp"

struct MarketUpdate {
    uint64_t timestamp;
    double price;
    double quantity;
    char symbol[16];
    char side; // 'B' for bid, 'A' for ask

    MarketUpdate() = default;
    MarketUpdate(uint64_t ts, double p, double q, std::string& sym, char s) :
        timestamp(ts), price(p), quantity(q), side(s) {
        strncpy(symbol, sym.c_str(), sizeof(symbol) - 1);
        symbol[sizeof(symbol) - 1] = '\0';
    }
};

class MarketDataProcessor {
    public:
        MarketDataProcessor();
        void start();
        void stop();

    private:
        static constexpr size_t BUFFER_SIZE = 1024;
        RingBuffer<MarketUpdate, BUFFER_SIZE> market_data_buffer_;

        bool running_{false};
        void producerThread();
        void consumerThread();
        void fetchData();
};
