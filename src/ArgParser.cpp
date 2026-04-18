#include "ArgParser.h"
#include <iostream>

ArgParser::ArgParser(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++)
    {
        std::string_view arg = argv[i];
        auto eq_sign_pos = arg.find('=');

        if (eq_sign_pos == std::string_view::npos || eq_sign_pos == 0 || eq_sign_pos == arg.size() - 1)
        {
            std::cerr << "dd: unrecognized operand: '" << arg << "'\n"
                << "Try 'dd --help' for more information.\n";
            throw std::invalid_argument("no = sign found");
        }

        // split on '='
        std::string key(arg.substr(0, eq_sign_pos));        
        std::string value(arg.substr(eq_sign_pos + 1));
        operandMap.emplace(std::move(key), std::move(value));
    }
}

static std::size_t handle_suffixes(const std::string value)
{
    return std::stoull(value); // TODO: handle k, M, G...
}

Arguments ArgParser::parse() const
{
    Arguments args;

    for (const auto& [operand, value] : operandMap)
    {
        if (operand == "if")
            args.inputFilename = value;
        else if (operand == "of")
            args.outputFilename = value;
        else if (operand == "ibs")
            args.inputBlockSize = handle_suffixes(value);
        else if (operand == "obs")
            args.outputBlockSize = handle_suffixes(value);
        else if (operand == "bs")
            args.inputBlockSize = args.outputBlockSize = handle_suffixes(value);
        else if (operand == "cbs")
            args.conversionBlockSize = handle_suffixes(value);
        else if (operand == "skip")
            args.skip = handle_suffixes(value);
        else if (operand == "iseek")
            args.inputSeek = handle_suffixes(value);
        else if (operand == "seek")
            args.seek = handle_suffixes(value);
        else if (operand == "oseek")
            args.outputSeek = handle_suffixes(value);
        else if (operand == "count")
            args.count = handle_suffixes(value);
        else if (operand == "status")
            args.status = value;
        else if (operand == "conv")
            args.conversion = value;
        else if (operand == "iflag")
            args.inputFlags = value;
        else if (operand == "oflag")
            args.outputFlags = value;
        else
        {
            std::cerr << "dd: unrecognized operand: '" << operand << "'\n"
                << "Try 'dd --help' for more information.\n";
            throw std::invalid_argument("operand not recognized");
        }
    }

    std::cout << args.status << '\n';
    return args;
}