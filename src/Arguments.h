#pragma once

#include <string>

#define IS_SET(x, flag) x & flag

enum Conversion : short
{
    ASCII = 1, // Convert EBCDIC to ASCII, using the conversion table specified by POSIX. This provides a 1:1 translation for all 256 bytes.
    EBCDIC = 1 << 1, // This is the inverse of the ‘ascii’ conversion.
    IBM = 1 << 2, // This acts like ‘conv=ebcdic’, except it uses the alternate conversion table specified by POSIX. 
    BLOCK = 1 << 3, // For each line in the input, output ‘cbs’ bytes, replacing the input newline with a space and truncating or padding input lines with spaces as necessary.
    UNBLOCK = 1 << 4, // Remove any trailing spaces in each ‘cbs’-sized input block, and append a newline.
    LCASE = 1 << 5, // Change uppercase letters to lowercase.
    UCASE = 1 << 6, // Change lowercase letters to uppercase. The ‘lcase’ and ‘ucase’ conversions are mutually exclusive.
    SPARSE = 1 << 7, // Try to seek rather than write NUL output blocks.
    SWAB = 1 << 8, // Swap every pair of input bytes.
    SYNC = 1 << 9, // Pad every input block to size of ‘ibs’ with trailing zero bytes. When used with ‘block’ or ‘unblock’, pad with spaces instead of zero bytes.
    EXCL = 1 << 10, // Fail if the output file already exists; dd must create the output file itself.
    NOCREAT = 1 << 11, // Do not create the output file; the output file must already exist.
    NOTRUNC = 1 << 12, // Do not truncate the output file.
    NOERR = 1 << 13, // Continue after read errors.
    FDATASYNC = 1 << 14, // Synchronize output data just before finishing, even if there were write errors. This forces a physical write of output data, so that even if power is lost the output data will be preserved.
    FSYNC = 1 << 15 // Synchronize output data and metadata just before finishing, even if there were write errors.
};

struct Arguments
{
    std::string inputFilename; // Read from file instead of standard input (if specified).
    std::string outputFilename; // Write to file instead of standard output. Unless ‘conv=notrunc’ is given, truncate file before writing it.

    // all numerical values are in bytes
    std::size_t inputBlockSize = 512; // Set the input block size to bytes. This makes dd read bytes per block.
    std::size_t outputBlockSize = 512; // Set the output block size to bytes. This makes dd write bytes per block.
    std::size_t conversionBlockSize = 0; // Set the conversion block size to bytes.
    
    std::size_t inputSeek = 0; // Skip n ‘ibs’-byte blocks in the input file before copying.
    std::size_t outputSeek = 0; // Skip n ‘obs’ - byte blocks in the output file before truncating or copying
    
    std::size_t count = 0; // Copy n ‘ibs’-byte blocks from the input file, instead of everything until the end of the file.
    std::string status = "none"; // Specify the amount of information printed. If this operand is given multiple times, the last one takes precedence.
    
    short conversions; // Convert the file as specified by the conversion argument(s).
    std::string inputFlags; // Access the input file using the flags specified by the flag argument(s).
    std::string outputFlags; // Access the output file using the flags specified by the flag argument(s).
};