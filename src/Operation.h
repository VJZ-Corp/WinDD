#pragma once

#include <string>
#include <variant>

struct StandardIn { /* represents read from stdin */ };
struct StandardOut { /* represents write to stdout */ };

// sources can be standard streams or file path (string)
using InputSource = std::variant<StandardIn, std::string>;
using OutputSource = std::variant<StandardOut, std::string>;

struct Operation 
{
    InputSource input = StandardIn{};
    OutputSource output = StandardOut{};

    std::size_t blockSize = 4096; // bs
    std::size_t inputBlockSize = 0; // ibs (0 = use bs)
    std::size_t outputBlockSize = 0; // obs (0 = use bs)
    std::size_t convBlockSize = 0; // cbs (rare, optional extension)

    std::size_t count = 0; // # of blocks (0 = unlimited)
    std::size_t skip = 0; // input block offset
    std::size_t seek = 0; // output block offset

    // TODO later: upgrade to enum bitmask if needed
    std::string conv;
    std::string inputFlags;
    std::string outputFlags;

    std::string status = "none";
};