#pragma once
#include <chrono>

void Benchmark(std::function<void()> func)
{
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = end - start;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(diff).count();

    std::cout << "Elapsed time: " << seconds << " seconds" << std::endl;
}
