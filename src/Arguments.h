#pragma once

#include <string>

struct Arguments
{
    std::string inputFilename; // Read from file instead of standard input (if specified).
    std::string outputFilename; // Write to file instead of standard output. Unless ‘conv=notrunc’ is given, truncate file before writing it.

    // all numerical values are in bytes
    std::size_t inputBlockSize = 512; // Set the input block size to bytes. This makes dd read bytes per block.
    std::size_t outputBlockSize = 512; // Set the output block size to bytes. This makes dd write bytes per block.
    std::size_t blockSize = 512; // Set both input and output block sizes to bytes: ibs and obs are overwritten.
    std::size_t conversionBlockSize = 0; // Set the conversion block size to bytes.
    
    std::size_t skip = 0;
    std::size_t inputSeek = 0; // Skip n ‘ibs’-byte blocks in the input file before copying.
    std::size_t seek = 0;
    std::size_t outputSeek = 0; // Skip n ‘obs’ - byte blocks in the output file before truncating or copying
    std::size_t count = 0; // Copy n ‘ibs’-byte blocks from the input file, instead of everything until the end of the file.

    std::string status = "none"; // Specify the amount of information printed. If this operand is given multiple times, the last one takes precedence.
    std::string conversion; // Convert the file as specified by the conversion argument(s).
    std::string inputFlags; // Access the input file using the flags specified by the flag argument(s).
    std::string outputFlags; // Access the output file using the flags specified by the flag argument(s).
};