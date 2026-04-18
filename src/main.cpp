#include <iostream>
#include "ArgParser.h"
#include "CopyEngine.h"

static void print_help()
{
    // TODO: implement help menu later
}

int main(int argc, char* argv[])
{
    // scan for options (of which there are only two)
    for (int i = 1; i < argc; i++)
    {
        std::string_view arg = argv[i];
        if (arg == "--help")
        {
            print_help();
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

    try
    {
        ArgParser arg_parser(argc, argv);
        Arguments args = arg_parser.parse();

        CopyEngine engine(args);
        engine.run();
    }
    catch (std::invalid_argument& ex)
    {
        return 1;
    }

    return 0;
}