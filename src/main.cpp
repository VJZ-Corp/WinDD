#include <iostream>
#include <unordered_map>
#include <unordered_set>

int main(int argc, char* argv[])
{
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
            std::cout << "dd (WinDD) 0.1\n"
                << "Copyright (C) 2026 VJZ Corporation.\n"
                << "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n"
                << "This is free software: you are free to change and redistribute it.\n"
                << "There is NO WARRANTY, to the extent permitted by law.\n\n"
                << "Written by bootsareme and tested by members of VJZ.\n";
            return 0;
        }
    }

    std::unordered_map<std::string, std::string> operands;
    const std::unordered_set<std::string_view> allowed_operands = {
        "bs", "cbs", "conv", "count", "ibs", "if", "iflag", "obs", 
        "of", "oflag", "seek", "skip", "status"
    };

    // parse operands
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

    for (const auto& [k, v] : operands) {
        std::cout << k << " = " << v << "\n";
    }

    return 0;
}