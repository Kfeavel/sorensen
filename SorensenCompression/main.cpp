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
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include "Stopwatch.hpp"


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

static void WriteVectorBool(std::fstream& fileOut, std::vector<bool>* x)
{
    for (size_t i = 0; i < x->size(); i++) {
        uint8_t byte = 0;
        for (uint8_t mask = 1; mask > 0; mask <<= 1) {
            if (x->at(i)) {
                byte |= mask;
            }
        }
        
        fileOut.write(reinterpret_cast<const char*>(&byte), sizeof(uint8_t));
    }
}

int main(int argc, char* const argv[])
{
    int ret = EXIT_SUCCESS;
    std::fstream fileIn;
    std::fstream fileOut;
    
    int8_t opt = 0;
    bool doCompress = false;
    while ((opt = getopt(argc, argv, "cdi:o:")) != -1) {
        switch (opt) {
            case 'c':
                doCompress = true;
                break;
            case 'd':
                doCompress = false;
                break;
            case 'i':
                fileIn.open(optarg, std::fstream::in | std::fstream::binary);
                if (!fileIn.is_open() || !fileIn.good()) {
                    std::cout << "Failed to open input file" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            case 'o':
                fileOut.open(optarg, std::fstream::out | std::fstream::binary);
                if (!fileOut.is_open() || !fileOut.good()) {
                    std::cout << "Failed to open output file" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            case 'h':
            default:
                std::cout << "TODO: Usage" << std::endl;
                return EXIT_SUCCESS;
         }
    }
    
    std::vector<bool>* bitset = new std::vector<bool>;
    CompressionData data = {
        .setBitsCountTotal = 0,
        .byteCountTotal = 0,
        .hash = 0,
    };
    
    if (doCompress) {
        std::cout << "Compressing..." << std::endl;
        Stopwatch([&]() {
            uint8_t byte = 0;
            while (fileIn.read(reinterpret_cast<char*>(&byte), sizeof(byte)))
            {
                data.byteCountTotal++;
                for (uint8_t i = 0; i < CHAR_BIT; i++)
                {
                    bitset->push_back(((byte >> i) & 1) != 0);
                    if (((byte >> i) & 1) != 0) {
                        data.setBitsCountTotal++;
                    }
                }
            }
            
            data.hash = std::hash<std::vector<bool>>{}(*bitset);
            fileOut.write(reinterpret_cast<const char*>(&data), sizeof(data));
            std::cout << "Done!" << std::endl;
        });
    } else {
        // Get data about the resulting file before performing the decompression so that we can
        // properly warn the end user that decompression may eat most of their RAM, and/or take
        // a literal eternity to decompress. For this reason chickening out is the default answer.
        
        if (GetFileSizeFromStream(fileIn) != sizeof(data))
        {
            std::cout << "Compression data is corrupt";
            ret = EXIT_FAILURE;
            goto fail;
        }
        
        fileIn.read(reinterpret_cast<char*>(&data), sizeof(data));
        if (!fileIn.good()) {
            std::cout << "Failed to read compression data";
            ret = EXIT_FAILURE;
            goto fail;
        }
        
        // TODO: Not quite accurate yet. Need time for 6 bytes to get better equation.
        double approximateSeconds = (45.576 * std::pow(data.byteCountTotal, 2)) - (209.28 * data.byteCountTotal) + 190.95;
        
        std::cout << "Decompression requires " << data.byteCountTotal << " bytes of memory." << std::endl;
        std::cout << "Approximate time to decompress: " << approximateSeconds << " seconds" << std::endl;
        std::cout << "Continue? [y/N] ";
        if (std::tolower(std::getchar()) != 'y') {
            ret = EXIT_FAILURE;
            goto fail;
        }
        
        std::cout << "Decompressing..." << std::endl;
        Stopwatch([&]() {
            // For `std::next_permutation` to work we need to fill in all of the zeros up front
            // and then fill in the remaining ones at the end. From there, `std::next_permutation`
            // will walk all possible iterations, effectively moving the ones from the last portion
            // of the bitset to the front.
            
            for (size_t i = 0; i < (data.byteCountTotal * CHAR_BIT) - data.setBitsCountTotal; i++) {
                bitset->push_back(0);
            }
            
            for (size_t i = 0; i < data.setBitsCountTotal; i++) {
                bitset->push_back(1);
            }
            
            // Perform the actual decompression
            do {
                size_t hash = std::hash<std::vector<bool>>{}(*bitset);
                if (hash == data.hash)
                {
                    std::cout << "Done!" << std::endl;
                    WriteVectorBool(fileOut, bitset);
                    break;
                }
            } while(std::next_permutation(bitset->begin(), bitset->end()));
        });
    }

fail:
    fileIn.close();
    fileOut.close();
    delete bitset;
    
    return ret;
}
