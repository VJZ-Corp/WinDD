#include "util.h"
#include <iostream>
#include <unordered_map>

static void print_version()
{
    std::cout << "WinDD 0.1\n"
        << "Copyright(C) 2026 VJZ Corporation\n"
        << "License GPLv3 + : GNU GPL version 3 or later < https ://gnu.org/licenses/gpl.html>.\n"
        << "This is free software : you are free to change and redistribute it.\n"
        << "There is NO WARRANTY, to the extent permitted by law.\n\n"
        << "Written by bootsareme and tested by members of VJZ.\n";
}

int main(int argc, char* argv[])
{
    std::unordered_map<std::string, std::string> operands;

    // scan for options (of which there are only two)
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
            print_version();
            return 0;
        }
    }

    // parse operands
    for (int i = 1; i < argc; i++)
    {
        std::string_view arg = argv[i];

        auto pos = arg.find('=');
        if (pos == std::string_view::npos || pos == 0 || pos == arg.size() - 1) 
        {
            std::cerr << "dd: unrecognized operand: '" << arg << "'\n"
                << "Try 'dd --help' for more information.\n";
            return 1;
        }

        std::string key(arg.substr(0, pos));
        std::string val(arg.substr(pos + 1));
        operands.emplace(std::move(key), std::move(val));
    }

    for (const auto& [k, v] : operands) {
        std::cout << k << " = " << v << "\n";
    }

    return 0;
}