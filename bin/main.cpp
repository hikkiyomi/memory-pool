#include "../include/PoolManager.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <list>
#include <utility>

class Object {
public:
    Object(int value)
        : value_(value)
    {}

    static void* operator new(size_t n) {
        return alloc_.allocate(n);
    }

    static void operator delete(void* ptr, size_t size) noexcept {
        alloc_.deallocate(reinterpret_cast<int*>(ptr), size);
    }

    int Get() const {
        return value_;
    }
private:
    static PoolManager<int> alloc_;
    int value_;
};

PoolManager<int> Object::alloc_{};

int main(int, char**) {
    std::ofstream stream("PoolManager-new.txt");
    std::vector<std::pair<int, double>> test;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 1; i <= 1000000; ++i) {
        Object* obj = new Object(10);

        auto cur = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = cur - start;

        test.push_back({i, duration.count()});
        delete obj;
    }

    for (auto [iter, passed] : test) {
        stream << iter << " " << std::fixed << std::setprecision(3) << passed << "\n";
    }

    // std::cout << "Took " << std::fixed << std::setprecision(3) << duration.count() << " seconds." << std::endl;

    return 0;
}
