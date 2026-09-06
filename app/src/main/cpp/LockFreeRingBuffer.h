#pragma once

#include <vector>
#include <atomic>
#include <cstddef>
#include <algorithm>

template<typename T>
class LockFreeRingBuffer {
public:
    explicit LockFreeRingBuffer(size_t capacity)
            : mBuffer(capacity), mCapacity(capacity), mHead(0), mTail(0) {}

    bool push(const T *data, size_t count) {
        size_t currentTail = mTail.load(std::memory_order_relaxed);
        size_t currentHead = mHead.load(std::memory_order_acquire);

        size_t available = mCapacity - (currentTail - currentHead); // cap - unread items
        if (count > available) {
            return false; // Buffer overflow
        }

        size_t index = currentTail % mCapacity;
        size_t firstPart = std::min(count, mCapacity - index);

        std::copy(data, data + firstPart, mBuffer.begin() + index);
        std::copy(data + firstPart, data + count, mBuffer.begin());

        mTail.store(currentTail + count, std::memory_order_release);
        return true;
    }

    bool pop(T *out, size_t count) {
        size_t currentHead = mHead.load(std::memory_order_relaxed);
        size_t currentTail = mTail.load(std::memory_order_acquire);

        if (currentHead == currentTail) {
            return false; // Underflow
        }

        size_t available = currentTail - currentHead;
        size_t toRead = std::min(count, available);

        size_t index = currentHead % mCapacity;
        size_t firstPart = std::min(toRead, mCapacity - index);

        std::copy(mBuffer.begin() + index, mBuffer.begin() + index + firstPart, out);
        std::copy(mBuffer.begin(), mBuffer.begin() + (toRead - firstPart), out + firstPart);

        mHead.store(currentHead + toRead, std::memory_order_release);
        return true;
    }

private:
    std::vector<T> mBuffer;
    size_t mCapacity;
    std::atomic<size_t> mHead;
    std::atomic<size_t> mTail;
};