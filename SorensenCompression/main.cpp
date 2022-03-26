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


int main(int argc, char* const argv[])
{
    std::fstream fileIn;
    
    int8_t opt = 0;
    bool doCompress = true;
    while ((opt = getopt(argc, argv, "cuf:")) != -1) {
        switch (opt) {
            case 'c':
                doCompress = true;
                break;
            case 'u':
                doCompress = false;
                break;
            case '?':
            default:
                std::cout << "TODO: Usage" << std::endl;
                return EXIT_SUCCESS;
         }
    }
    
    std::cout << "File path: " << std::endl;
    std::string path;
    std::getline(std::cin, path);
    if (path.empty()) {
        return EXIT_FAILURE;
    }
    
    std::cout << "Opening '" << path << "'" << std::endl;
    fileIn.open(path, std::fstream::in | std::fstream::out | std::fstream::binary);
    if (!fileIn.is_open()) {
        std::cout << "Failed to open file" << std::endl;
        return EXIT_FAILURE;
    }
    
    if (!fileIn.good()) {
        std::cout << "File is not good" << std::endl;
        return EXIT_FAILURE;
    }
    
    if (doCompress) {
        char byte;
        uint16_t bitCountTotal = 0;
        while (fileIn.read(&byte, sizeof(byte)))
        {
            uint16_t bitCountCurr = 0;
            std::cout << std::bitset<CHAR_BIT>(byte).to_string() << "\t";
            
            while (byte) {
                if (byte % 2 == 1) {
                    bitCountCurr++;
                    bitCountTotal++;
                }
                
                byte = byte >> 1;
            }
            
            std::cout << "Current: " << bitCountCurr << " Total: " << bitCountTotal << std::endl;
        }
    } else {
        std::cout << "TODO: Decompression" << std::endl;
    }

    fileIn.close();
    return EXIT_SUCCESS;
}
