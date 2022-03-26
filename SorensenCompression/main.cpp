//
//  main.cpp
//  SorensenCompression
//
//  Created by Keeton Feavel on 3/26/22.
//

#include <bitset>
#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>


struct CompressionData
{
    uint64_t bitCountTotal;     // Total number of set bits
    uint64_t byteCountTotal;    // Total number of bytes
};


int main(int argc, char* const argv[])
{
    std::fstream fileIn;
    std::fstream fileOut;
    
    int8_t opt = 0;
    bool doCompress = false;
    while ((opt = getopt(argc, argv, "cui:o:")) != -1) {
        switch (opt) {
            case 'c':
                doCompress = true;
                break;
            case 'u':
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
    
    if (doCompress) {
        CompressionData data = {
            .bitCountTotal = 0,
            .byteCountTotal = 0,
        };
        
        char byte;
        while (fileIn.read(&byte, sizeof(byte)))
        {
            data.byteCountTotal++;
            while (byte) {
                if (byte % 2 == 1) {
                    data.bitCountTotal++;
                }
                
                byte = byte >> 1;
            }
        }
        
        std::cout << "Writing results..." << std::endl;
        fileOut.write(reinterpret_cast<const char*>(&data), sizeof(data));
    } else {
        std::cout << "TODO: Decompression" << std::endl;
    }

    std::cout << "Done!" << std::endl;
    fileIn.close();
    fileOut.close();
    
    return EXIT_SUCCESS;
}
