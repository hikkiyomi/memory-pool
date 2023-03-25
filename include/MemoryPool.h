#pragma once

#include <cinttypes>
#include <stdexcept>

template<typename T>
class PoolAllocator {
public:
    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;
public:
    PoolAllocator(
        uint32_t amount_of_blocks = 4096,
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

    ~PoolAllocator() {
        delete[] data_begin_;
    }

    pointer allocate(size_t n) {
        size_t total_size = n * sizeof(T);
        size_t blocks_needed = TakenBlocks(total_size);

        for (size_t i = 0; i < blocks_needed; ++i) {
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
        size_t total_size = n * sizeof(T);
        size_t freed_blocks = TakenBlocks(total_size);
        uint8_t* current_block = Shift(reinterpret_cast<uint8_t*>(ptr) - kOffset, freed_blocks - 1);

        for (size_t i = freed_blocks; i > 0; --i) {
            size_t next_index = num_of_blocks_;

            if (next_) {
                next_index = GetIndexFromAddress(next_);
            }

            *reinterpret_cast<uint32_t*>(current_block) = next_index;
            next_ = reinterpret_cast<uint8_t*>(current_block);
            current_block -= real_size_of_block_;
        }

        free_blocks_ += freed_blocks;
    }
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
    uint8_t* GetAddressFromIndex(uint32_t index) noexcept {
        return data_begin_ + (index * real_size_of_block_);
    }

    uint32_t GetIndexFromAddress(const uint8_t* ptr) const noexcept {
        return static_cast<uint32_t>(ptr - data_begin_) / real_size_of_block_;
    }

    size_t TakenBlocks(size_t total_size) const noexcept {
        return (total_size + size_of_block_ - 1) / size_of_block_;
    }

    uint8_t* Shift(uint8_t* start, size_t blocks) const noexcept {
        return start + blocks * real_size_of_block_;
    }

    uint8_t* GetSuitable(size_t blocks) {
        uint8_t* prev_free = nullptr;
        uint8_t* cur_row = next_;
        uint8_t* cur_block = next_;
        uint8_t* next_block = nullptr;
        size_t counter = 0;

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
