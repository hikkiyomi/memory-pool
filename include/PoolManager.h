#pragma once

#include "MemoryPool.h"

#include <queue>

template<typename T>
class PoolManager : public InnerTypes<T> {
public:
    using value_type      = typename InnerTypes<T>::value_type;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using size_type       = typename InnerTypes<T>::size_type;
    using difference_type = typename InnerTypes<T>::difference_type;
public:
    PoolManager() = default;
    PoolManager(const PoolManager& other) = delete;
    PoolManager& operator=(const PoolManager& other) = delete;
    PoolManager(PoolManager&& other) = delete;
    PoolManager& operator=(PoolManager&& other) = delete;
    
    ~PoolManager() = default;
public:
    bool operator==(const PoolManager&) noexcept {
        return true;
    }

    bool operator!=(const PoolManager&) noexcept {
        return false;
    }
public:
    [[nodiscard]] pointer allocate(size_type n) {
        using Pair = std::pair<uint32_t, PoolAllocator<T>*>;

        std::priority_queue<
                        Pair,
                        std::vector<Pair>,
                        std::greater<Pair>
        > pq;

        size_type total_size = n * sizeof(T);

        for (auto& pool : pools_) {
            pq.push({pool.GetSurplusMemory(total_size), &pool});
        }

        pointer res = nullptr;

        while (!pq.empty()) {
            auto [surplus, alloc_ptr] = pq.top();
            pq.pop();

            try {
                res = alloc_ptr->allocate(n);
            } catch (const std::bad_alloc& e) {}
        }

        if (!res) {
            throw std::bad_alloc();
        }

        return res;
    }

    void deallocate(pointer ptr, size_type n) noexcept {
        for (auto& pool : pools_) {
            if (pool.IsIn(ptr)) {
                pool.deallocate(ptr, n);
                break;
            }
        }
    }
private:
    inline static const size_t kPoolsAmount = 1;

    PoolAllocator<T> pools_[kPoolsAmount] = {
        PoolAllocator<T>(400000, sizeof(T)),
    };
};
