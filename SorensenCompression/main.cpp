//
//  main.cpp
//  SorensenCompression
//
//  Created by Keeton Feavel on 3/26/22.
//

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
    uint64_t bitCountTotal;     // Total number of set bits
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

static void binary_write(std::fstream& fout, std::vector<bool>* x)
{
    std::vector<bool>::size_type i = 0;
    std::vector<bool>::size_type n = x->size();
    while (i < n) {
        uint8_t aggr = 0;
        for (uint8_t mask = 1; mask > 0 && i < n; ++i, mask <<= 1) {
            if (x->at(i)) {
                aggr |= mask;
            }
        }
        
        fout.write(reinterpret_cast<const char*>(&aggr), sizeof(uint8_t));
    }
}

int main(int argc, char* const argv[])
{
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
    if (doCompress) {
        Stopwatch([&]() {
            CompressionData data = {
                .bitCountTotal = 0,
                .byteCountTotal = 0,
                .hash = 0,
            };
            
            uint8_t byte = 0;
            while (fileIn.read(reinterpret_cast<char*>(&byte), sizeof(byte)))
            {
                data.byteCountTotal++;
                for (uint8_t i = 0; i < CHAR_BIT; i++)
                {
                    bitset->push_back(((byte >> i) & 1) != 0);
                    if (((byte >> i) & 1) != 0) {
                        data.bitCountTotal++;
                    }
                }
            }
            
            data.hash = std::hash<std::vector<bool>>{}(*bitset);
            fileOut.write(reinterpret_cast<const char*>(&data), sizeof(data));
        });
    } else {
        Stopwatch([&]() {
            CompressionData data = {
                .bitCountTotal = 0,
                .byteCountTotal = 0,
                .hash = 0,
            };
            
            if (GetFileSizeFromStream(fileIn) != sizeof(data))
            {
                std::cout << "Compression data is corrupt";
                exit(EXIT_FAILURE);
            }
            
            // FIXME: How should I error check this?
            fileIn.read(reinterpret_cast<char*>(&data), sizeof(data));
            
            for (size_t i = 0; i < (data.byteCountTotal * CHAR_BIT) - data.bitCountTotal; i++) {
                bitset->push_back(0);
            }
            
            for (size_t i = 0; i < data.bitCountTotal; i++) {
                bitset->push_back(1);
            }
            
            std::cout << "Decompressing... (This may take a while)" << std::endl;
            do {
                size_t hash = std::hash<std::vector<bool>>{}(*bitset);
                if (hash == data.hash)
                {
                    std::cout << "Got it!" << std::endl;
                    binary_write(fileOut, bitset);
                    break;
                }
            } while(std::next_permutation(bitset->begin(), bitset->end()));
        });
    }

    fileIn.close();
    fileOut.close();
    delete bitset;
    
    return EXIT_SUCCESS;
}
