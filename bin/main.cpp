#include "../include/PoolManager.h"

#include <chrono>
#include <iostream>
#include <iomanip>
#include <list>

int main(int, char**) {
    std::list<int, PoolManager<int>> v;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 400000; ++i) {
        v.emplace_back(1);
    }

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = stop - start;

    std::cout << "Took " << std::fixed << std::setprecision(3) << duration.count() << " seconds." << std::endl;

    return 0;
}
