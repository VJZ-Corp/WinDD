#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "Operation.h"

static void dispatch(std::unordered_map<std::string, std::string>& operand_map)
{
    Operation op;

    for (const auto& [operand, value] : operand_map)
    {
        if (operand == "if")
            op.input = value; // file path
        else if (operand == "of")
            op.output = value;
        else if (operand == "bs")
            op.blockSize = std::stoull(value);
        else if (operand == "ibs")
            op.inputBlockSize = std::stoull(value);
        else if (operand == "obs")
            op.outputBlockSize = std::stoull(value);
        else if (operand == "cbs")
            op.convBlockSize = std::stoull(value);
        else if (operand == "count")
            op.count = std::stoull(value);
        else if (operand == "skip")
            op.skip = std::stoull(value);
        else if (operand == "seek")
            op.seek = std::stoull(value);
        else if (operand == "conv")
            op.conv = value;
        else if (operand == "iflag")
            op.inputFlags = value;
        else if (operand == "oflag")
            op.outputFlags = value;
        else if (operand == "status")
            op.status = value;
    }

    std::cout << op.status << '\n';
}

int main(int argc, char* argv[])
{
    // scan for tions (of which there are only two)
    for (int i = 1; i < argc; i++)
    {
        std::string_view arg = argv[i];
        if (arg == "--help")
        {
            // TODO: implement help menu later
            return 0; // --help takes precedence over --version
        }
        if (arg == "--version")
        {
            std::cout << "dd (WinDD) 0.1\n"
                      << "Cyright (C) 2026 VJZ Corporation.\n"
                      << "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n"
                      << "This is free software: you are free to change and redistribute it.\n"
                      << "There is NO WARRANTY, to the extent permitted by law.\n\n"
                      << "Written by bootsareme and tested by members of VJZ.\n";
            return 0;
        }
    }

    const std::unordered_set<std::string_view> allowed_operands = {
        "bs", "cbs", "conv", "count", "ibs", "if", "iflag", "obs", 
        "of", "oflag", "seek", "skip", "status"
    };
    std::unordered_map<std::string, std::string> operands;

    // parse erands
    for (int i = 1; i < argc; i++)
    {
        std::string_view arg = argv[i];
        auto eq_sign_pos = arg.find('=');

        if (eq_sign_pos == std::string_view::npos || eq_sign_pos == 0 || eq_sign_pos == arg.size() - 1) 
        {
            std::cerr << "dd: unrecognized operand: '" << arg << "'\n"
                      << "Try 'dd --help' for more information.\n";
            return 1;
        }

        // split on '='
        std::string key(arg.substr(0, eq_sign_pos));

        if (!allowed_operands.contains(key))
        {
            std::cerr << "dd: unrecognized operand: '" << arg << "'\n"
                      << "Try 'dd --help' for more information.\n";
            return 1;
        }

        std::string value(arg.substr(eq_sign_pos + 1));
        operands.emplace(std::move(key), std::move(value));
    }

    dispatch(operands);
    return 0;
}