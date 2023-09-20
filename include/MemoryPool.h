#pragma once

#include "InnerTypes.h"

#include <cinttypes>
#include <stdexcept>
#include <utility>

template<typename T>
class PoolAllocator : public InnerTypes<T> {
public:
    using value_type      = typename InnerTypes<T>::value_type;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using size_type       = typename InnerTypes<T>::size_type;
    using difference_type = typename InnerTypes<T>::difference_type;
public:
    explicit PoolAllocator(
        uint32_t amount_of_blocks = 1000000,
        uint32_t size_of_block = sizeof(T)
    )
        : num_of_blocks_(amount_of_blocks)
        , size_of_block_(size_of_block)
        , real_size_of_block_(size_of_block + kOffset)
        , free_blocks_(amount_of_blocks)
        , initialized_(0)
    {
        if (amount_of_blocks <= 0) {
            throw std::runtime_error("Amount of blocks must be at least 1");
        }

        if (size_of_block_ <= 0) {
            throw std::runtime_error("Size of block must be at least 1");
        }

        data_begin_ = new uint8_t[num_of_blocks_ * real_size_of_block_];
        next_ = data_begin_;
    }

    PoolAllocator(const PoolAllocator<T>& other) noexcept
        : num_of_blocks_(other.num_of_blocks_)
        , size_of_block_(other.size_of_block_)
        , real_size_of_block_(other.real_size_of_block_)
        , free_blocks_(other.num_of_blocks_)
        , initialized_(0)
        , data_begin_(other.data_begin_)
        , next_(other.data_begin_)
    {}

    PoolAllocator& operator=(const PoolAllocator<T>& other) noexcept {
        delete[] data_begin_;

        num_of_blocks_ = other.num_of_blocks_;
        size_of_block_ = other.size_of_block_;
        real_size_of_block_ = other.size_of_block_;
        free_blocks_ = other.num_of_blocks_;
        initialized_ = 0;
        data_begin_ = other.data_begin_;
        next_ = other.next_;

        return *this;
    }

    template<typename U>
    PoolAllocator(const PoolAllocator<U>&) {}

    PoolAllocator(PoolAllocator<T>&& other) = default;

    PoolAllocator& operator=(PoolAllocator<T>&& other) = default;

    template<typename U>
    PoolAllocator(PoolAllocator<U>&&) {};

    ~PoolAllocator() {
        delete[] data_begin_;
    }
public:
    bool operator==(const PoolAllocator<T>& other) noexcept {
        return num_of_blocks_ == other.num_of_blocks_
            && size_of_block_ == other.size_of_block_
            && data_begin_ == other.data_begin_;
    }

    bool operator!=(const PoolAllocator<T>& other) noexcept {
        return !(*this == other);
    }
public:
    [[nodiscard]] pointer allocate(size_type n) {
        size_type total_size = n * sizeof(T);
        size_type blocks_needed = TakenBlocks(total_size);

        for (size_type i = 0; i < blocks_needed; ++i) {
            if (initialized_ < num_of_blocks_) {
                uint32_t* number_of_next = reinterpret_cast<uint32_t*>(GetAddressFromIndex(initialized_));
                *number_of_next = initialized_ + 1;
                ++initialized_;
            } else {
                break;
            }
        }

        uint8_t* ret = nullptr;

        if (free_blocks_ < blocks_needed) {
            throw std::bad_alloc();
        }

        ret = GetSuitable(blocks_needed);

        if (!ret) {
            throw std::bad_alloc();
        }

        ret += kOffset;
        free_blocks_ -= blocks_needed;

        return reinterpret_cast<pointer>(ret);
    }

    void deallocate(pointer ptr, size_type n) noexcept {
        size_type total_size = n * sizeof(T);
        size_type freed_blocks = TakenBlocks(total_size);
        uint8_t* current_block = Shift(reinterpret_cast<uint8_t*>(ptr) - kOffset, freed_blocks - 1);

        for (size_type i = freed_blocks; i > 0; --i) {
            size_type next_index = num_of_blocks_;

            if (next_) {
                next_index = GetIndexFromAddress(next_);
            }

            *reinterpret_cast<uint32_t*>(current_block) = next_index;
            next_ = reinterpret_cast<uint8_t*>(current_block);
            current_block -= real_size_of_block_;
        }

        free_blocks_ += freed_blocks;
    }
public:
    uint32_t GetNumberOfBlocks() const noexcept {
        return num_of_blocks_;
    }

    uint32_t GetSizeOfBlocks() const noexcept {
        return size_of_block_;
    }

    uint32_t GetSurplusMemory(size_type total_size) const noexcept {
        return TakenBlocks(total_size) * size_of_block_ - total_size;
    }

    bool IsIn(pointer ptr) const noexcept {
        uint8_t* data_pointer = reinterpret_cast<uint8_t*>(ptr);
        return data_begin_ <= data_pointer 
            && data_pointer < data_begin_ + num_of_blocks_ * real_size_of_block_;
    };
private:
    static const uint32_t kOffset = 4;

    uint32_t num_of_blocks_;
    uint32_t size_of_block_;
    uint32_t real_size_of_block_;
    uint32_t free_blocks_;
    uint32_t initialized_;

    uint8_t* data_begin_;
    uint8_t* next_;
private:
    PoolAllocator(
        uint32_t amount_of_blocks,
        uint32_t size_of_block,
        uint32_t real_size_of_block,
        uint32_t free_blocks,
        uint32_t initialized,
        uint8_t* data_begin,
        uint8_t* next
    )
        : num_of_blocks_(amount_of_blocks)
        , size_of_block_(size_of_block)
        , real_size_of_block_(real_size_of_block)
        , free_blocks_(free_blocks)
        , initialized_(initialized)
        , data_begin_(data_begin)
        , next_(next)
    {}
private:
    uint8_t* GetAddressFromIndex(uint32_t index) noexcept {
        return data_begin_ + (index * real_size_of_block_);
    }

    uint32_t GetIndexFromAddress(const uint8_t* ptr) const noexcept {
        return static_cast<uint32_t>(ptr - data_begin_) / real_size_of_block_;
    }

    size_type TakenBlocks(size_type total_size) const noexcept {
        return (total_size + size_of_block_ - 1) / size_of_block_;
    }

    uint8_t* Shift(uint8_t* start, size_type blocks) const noexcept {
        return start + blocks * real_size_of_block_;
    }

    uint8_t* GetSuitable(size_type blocks) {
        uint8_t* prev_free = nullptr;
        uint8_t* cur_row = next_;
        uint8_t* cur_block = next_;
        uint8_t* next_block = nullptr;
        size_type counter = 0;

        while (counter < blocks) {
            uint32_t index_of_block = GetIndexFromAddress(cur_block);

            if (index_of_block >= num_of_blocks_) {
                return nullptr;
            }

            ++counter;
            next_block = GetAddressFromIndex(*reinterpret_cast<uint32_t*>(cur_block));

            if (next_block - cur_block != real_size_of_block_) {
                prev_free = cur_block;
                counter = 0;
                cur_row = next_block;
            }

            cur_block = next_block;
        }

        if (prev_free) {
            uint32_t next_index = num_of_blocks_;

            if (next_block) {
                next_index = GetIndexFromAddress(next_block);
            }

            *reinterpret_cast<uint32_t*>(prev_free) = next_index;
        } else {
            uint32_t next_index = num_of_blocks_;

            if (next_block) {
                next_index = GetIndexFromAddress(next_block);
            }

            if (next_index < num_of_blocks_) {
                next_ = GetAddressFromIndex(next_index);
            } else {
                next_ = nullptr;
            }
        }

        return cur_row;
    }
};
