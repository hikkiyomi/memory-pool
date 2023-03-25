#include "../include/MemoryPool.h"

#include <chrono>
#include <iostream>
#include <list>

int main(int, char**) {
    std::list<int, PoolAllocator<int>> v;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000000; ++i) {
        v.push_back(1);
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << duration.count() << std::endl;

    return 0;
}
