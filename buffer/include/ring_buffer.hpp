#pragma once
#include <atomic>
#include <array>
#include <cstddef>
#include <cstdint>

template<typename T, size_t Size>
class RingBuffer {
    private:
        static constexpr size_t CACHE_LINE_SIZE = 64;
        static constexpr size_t BUFFER_SIZE = Size;

        alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> head_{0};
        alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> tail_{0};

        alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> push_count_{0};
        alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> pop_count_{0};

        alignas(CACHE_LINE_SIZE) std::array<T, BUFFER_SIZE> buffer_;

    public:
        RingBuffer() = default;

        RingBuffer(const RingBuffer&) = delete;
        RingBuffer& operator=(RingBuffer&) = delete;
        RingBuffer(RingBuffer&&) = delete;
        RingBuffer& operator=(RingBuffer&&) = delete;


        bool push(const T& item) {
            uint64_t current_tail = tail_.load(std::memory_order_relaxed);
            uint64_t next_tail = (current_tail + 1) % BUFFER_SIZE;

            // Full Buffer Cond.
            if (next_tail == head_.load(std::memory_order_acquire))
                return false;

            buffer_[current_tail] = item;
            tail_.store(next_tail, std::memory_order_release);
            push_count_.fetch_add(1, std::memory_order_relaxed);
            return true;
        }

        bool pop(T& item) {
            uint64_t current_head = head_.load(std::memory_order_relaxed);

            // Buffer empty Cond.
            if (current_head == tail_.load(std::memory_order_acquire))
                return false;

            item = buffer_[current_head];
            head_.store((current_head + 1) % BUFFER_SIZE, std::memory_order_release);
            pop_count_.fetch_add(1, std::memory_order_relaxed);
            return true;
        }

        uint64_t pushCount() const {
            return push_count_.load(std::memory_order_relaxed);
        }

        size_t size() const {
            uint64_t head = head_.load(std::memory_order_relaxed);
            uint64_t tail = tail_.load(std::memory_order_relaxed);

            return tail >= head ? tail - head : BUFFER_SIZE - (head - tail);
        }


};
