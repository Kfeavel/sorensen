//
//  Stopwatch.hpp
//  SorensenCompression
//
//  Created by Keeton Feavel on 3/26/22.
//

#pragma once
#include <chrono>
#include <iostream>
#include <functional>


void Stopwatch(std::function<void()> func)
{
    auto start = std::chrono::steady_clock::now();
    func();
    auto end = std::chrono::steady_clock::now();
    
    std::cout
        << "Elapsed time: "
        << std::chrono::duration_cast<std::chrono::seconds>(end - start).count()
        << ":"
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << ":"
        << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
        << ":"
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << std::endl;
}
