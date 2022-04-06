//
//  main.cpp
//  SorensenCompression
//
//  Created by Keeton Feavel on 3/26/22.
//

#include <cmath>
#include <vector>
#include <bitset>
#include <string>
#include <chrono>
#include <climits>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unistd.h>


enum Mode {
    INVALID,
    COMPRESS,
    DECOMPRESS,
};

struct CompressionData
{
    uint64_t setBitsCountTotal; // Total number of set / high bits
    uint64_t byteCountTotal;    // Total number of bytes
    uint64_t hash;              // Hash of bitset
};

static std::streamsize GetFileSizeFromStream(std::fstream& file)
{
    file.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize length = file.gcount();
    file.clear(); // Since ignore will have set eof.
    file.seekg(0, std::ios_base::beg);
    return length;
}

static void WriteVectorBool(std::fstream& fileOut, std::vector<bool>& x)
{
    std::vector<bool>::size_type i = 0;
    std::vector<bool>::size_type n = x.size();
    while (i < n) {
        uint8_t aggr = 0;
        for (uint8_t mask = 1; mask > 0 && i < n; ++i, mask <<= 1) {
            if (x.at(i)) {
                aggr |= mask;
            }
        }

        fileOut.write(reinterpret_cast<const char*>(&aggr), sizeof(uint8_t));
    }
}

static void PrintElapsedTime(std::chrono::time_point<std::chrono::steady_clock>& start)
{
    auto end = std::chrono::steady_clock::now();
    std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;
}

static int Compress(std::fstream& fileIn, std::fstream& fileOut, CompressionData& data, std::vector<bool>& bitset)
{
    std::cout << "Compressing..." << std::endl;
    uint8_t byte = 0;
    while (fileIn.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        data.byteCountTotal++;
        for (uint8_t i = 0; i < CHAR_BIT; i++) {
            bitset.push_back(((byte >> i) & 1) != 0);
            if (((byte >> i) & 1) != 0) {
                data.setBitsCountTotal++;
            }
        }
    }

    data.hash = std::hash<std::vector<bool>>{}(bitset);
    fileOut.write(reinterpret_cast<const char*>(&data), sizeof(data));
    std::cout << "Done!" << std::endl;
    return EXIT_SUCCESS;
}

static int Decompress(std::fstream& fileIn, std::fstream& fileOut, CompressionData& data, std::vector<bool>& bitset)
{
    // Get data about the resulting file before performing the decompression so that we can
    // properly warn the end user that decompression may eat most of their RAM, and/or take
    // a literal eternity to decompress. For this reason chickening out is the default answer.

    if (GetFileSizeFromStream(fileIn) != sizeof(data)) {
        std::cout << "Compression data is corrupt";
        return EXIT_FAILURE;
    }

    fileIn.read(reinterpret_cast<char*>(&data), sizeof(data));
    if (!fileIn.good()) {
        std::cout << "Failed to read compression data";
        return EXIT_FAILURE;
    }

    double approximateSeconds = tgamma((data.setBitsCountTotal * 2) + 1) / pow(tgamma(data.setBitsCountTotal + 1), 2);
    double approximateYears = approximateSeconds / (60.0f * 60.0f * 24.0f * 365.0f);

    std::cout << "Decompression requires " << data.byteCountTotal << " bytes of memory." << std::endl;
    std::cout << "Approximate time to decompress: " << approximateYears << " year(s)" << std::endl;
    std::cout << "Continue? [y/N] ";
    if (std::tolower(std::getchar()) != 'y') {
        return EXIT_FAILURE;
    }

    std::cout << "Decompressing..." << std::endl;
    // For `std::next_permutation` to work we need to fill in all of the zeros up front
    // and then fill in the remaining ones at the end. From there, `std::next_permutation`
    // will walk all possible iterations, effectively moving the ones from the last portion
    // of the bitset to the front.

    for (size_t i = 0; i < (data.byteCountTotal * CHAR_BIT) - data.setBitsCountTotal; i++) {
        bitset.push_back(0);
    }

    for (size_t i = 0; i < data.setBitsCountTotal; i++) {
        bitset.push_back(1);
    }

    // Perform the actual decompression
    uint64_t iterations = 0;
    auto start = std::chrono::steady_clock::now();
    do {
        if (++iterations % UINT64_MAX == 0) {
            std::cout << "Still working on it..." << std::endl;
            PrintElapsedTime(start);
        }

        size_t hash = std::hash<std::vector<bool>>{}(bitset);
        if (hash == data.hash) {
            std::cout << "Done!" << std::endl;
            WriteVectorBool(fileOut, bitset);
            break;
        }
    } while(std::next_permutation(bitset.begin(), bitset.end()));

    return EXIT_SUCCESS;
}

void Usage(char* const argv[]) {
    std::cout
        << "Usage: " << argv[0] << " action input output" << std::endl
        << "action\tMust be either `compress` or `decompress`" << std::endl
        << "input\tInput file path" << std::endl
        << "output\tOutput file path" << std::endl;
}

int main(int argc, char* const argv[])
{
    if (argc != 4) {
        Usage(argv);
        return EXIT_FAILURE;
    }

    // argv[1] is always mode
    Mode mode = INVALID;
    std::string argMode = argv[1];
    if (argMode == "compress") {
        mode = COMPRESS;
    } else if (argMode == "decompress") {
        mode = DECOMPRESS;
    } else {
        Usage(argv);
        return EXIT_FAILURE;
    }

    // argv[2] is always input
    std::fstream fileIn;
    fileIn.open(argv[2], std::fstream::in | std::fstream::binary);
    if (!fileIn.is_open() || !fileIn.good()) {
        std::cout << "Failed to open input file" << std::endl;
        return EXIT_FAILURE;
    }

    // argv[3] is always output
    std::fstream fileOut;
    fileOut.open(argv[3], std::fstream::out | std::fstream::trunc | std::fstream::binary);
    if (!fileOut.is_open() || !fileOut.good()) {
        std::cout << "Failed to open output file" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<bool> bitset;
    CompressionData data = {
        .setBitsCountTotal = 0,
        .byteCountTotal = 0,
        .hash = 0,
    };

    int err = EXIT_FAILURE;
    switch (mode)
    {
        case COMPRESS:
            err = Compress(fileIn, fileOut, data, bitset);
            break;
        case DECOMPRESS:
            err = Decompress(fileIn, fileOut, data, bitset);
            break;
        default:
            Usage(argv);
            break;
    }

    return err;
}
