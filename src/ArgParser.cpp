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

static std::size_t handle_suffixes(const std::string& value)
{
    // split numeric part and suffix
    std::size_t i = 0;
    while (i < value.size() && std::isdigit(value[i]))
        ++i;

    if (i == 0)
    {
        std::cerr << "dd: invalid number: '" << value << "'\n";
        throw std::invalid_argument("Invalid numeric value");
    }

    std::size_t result = std::stoull(value.substr(0, i));
    if (result == 0)
    {
        std::cerr << "dd: invalid number: '" << value << "'\n";
        throw std::invalid_argument("scalar must be non-zero");
    }
    
    if (value.compare(i, 2, "kB") == 0)
        return result * 1000ULL;
    else if (value.compare(i, 2, "MB") == 0)
        return result * 1000ULL * 1000;
    else if (value.compare(i, 2, "GB") == 0)
        return result * 1000ULL * 1000 * 1000;
    else if (value.compare(i, 2, "TB") == 0)
        return result * 1000ULL * 1000 * 1000 * 1000;
    else if (value.compare(i, 2, "PB") == 0)
        return result * 1000ULL * 1000 * 1000 * 1000 * 1000;
    else if (value.compare(i, 2, "EB") == 0)
        return result * 1000ULL * 1000 * 1000 * 1000 * 1000 * 1000;
    else if (value.compare(i, 3, "KiB") == 0)
        return result * (1ULL << 10);
    else if (value.compare(i, 3, "MiB") == 0)
        return result * (1ULL << 20);
    else if (value.compare(i, 3, "GiB") == 0)
        return result * (1ULL << 30);
    else if (value.compare(i, 3, "TiB") == 0)
        return result * (1ULL << 40);
    else if (value.compare(i, 3, "PiB") == 0)
        return result * (1ULL << 50);
    else if (value.compare(i, 3, "EiB") == 0)
        return result * (1ULL << 60);
    else
    {            
        if (i == value.size())
            return result; // no suffix found

        short exp = 0;
        switch (value[i])
        {
        case 'B':
            break;
        case 'k':
            exp = 10;
            break;
        case 'M':
            exp = 20;
            break;
        case 'G':
            exp = 30;
            break;
        case 'T':
            exp = 40;
            break;
        case 'P':
            exp = 50;
            break;
        case 'E':
            exp = 60;
            break;
        default:
            std::cerr << "dd: invalid number: '" << value << "'\n";
            throw std::invalid_argument("invalid suffix");
        }

        return result * (1ULL << exp);
    }
}

static short parse_flags(std::string_view input, bool is_conv)
{
    short flags = 0; // no flags
    size_t start = 0;

    while (start < input.size())
    {
        size_t end = input.find(',', start);
        if (end == std::string_view::npos)
            end = input.size();

        std::string_view flag = input.substr(start, end - start);

        if (is_conv)
        {
            if (flag == "ascii")
                flags |= Conversion::ASCII;
            else if (flag == "ebcdic")
                flags |= Conversion::EBCDIC;
            else if (flag == "ibm")
                flags |= Conversion::IBM;
            else if (flag == "block")
                flags |= Conversion::BLOCK;
            else if (flag == "unblock")
                flags |= Conversion::UNBLOCK;
            else if (flag == "lcase")
                flags |= Conversion::LCASE;
            else if (flag == "ucase")
                flags |= Conversion::UCASE;
            else if (flag == "sparse")
                flags |= Conversion::SPARSE;
            else if (flag == "swab")
                flags |= Conversion::SWAB;
            else if (flag == "sync")
                flags |= Conversion::SYNC;
            else if (flag == "excl")
                flags |= Conversion::EXCL;
            else if (flag == "nocreat")
                flags |= Conversion::NOCREAT;
            else if (flag == "notrunc")
                flags |= Conversion::NOTRUNC;
            else if (flag == "noerror")
                flags |= Conversion::NOERR;
            else if (flag == "fdatasync")
                flags |= Conversion::FDATASYNC;
            else if (flag == "fsync")
                flags |= Conversion::FSYNC;
            else
            {
                std::cerr << "dd: invalid conversion: '" << flag << "'\n"
                          << "Try 'dd --help' for more information.\n";
                throw std::invalid_argument("conversion not recognized");
            }

            start = end + 1;
        }
    }

    // handle mutually exclusive flags
    if (is_conv && IS_SET(flags, Conversion::LCASE) && IS_SET(flags, Conversion::UCASE))
    {
        std::cerr << "dd: cannot combine lcase and ucase\n";
        throw std::invalid_argument("mutually exclusive conversions");

    }
    else if (is_conv && IS_SET(flags, Conversion::EXCL) && IS_SET(flags, Conversion::NOCREAT))
    {
        std::cerr << "dd: cannot combine excl and nocreat\n";
        throw std::invalid_argument("mutually exclusive conversions");

    }
    else if (is_conv && IS_SET(flags, Conversion::ASCII) && IS_SET(flags, Conversion::EBCDIC)
        || IS_SET(flags, Conversion::ASCII) && IS_SET(flags, Conversion::IBM)
        || IS_SET(flags, Conversion::EBCDIC) && IS_SET(flags, Conversion::IBM)
        || IS_SET(flags, Conversion::ASCII) && IS_SET(flags, Conversion::EBCDIC) && IS_SET(flags, Conversion::IBM))
    {
        std::cerr << "dd: cannot combine any two of {ascii,ebcdic,ibm}\n";
        throw std::invalid_argument("mutually exclusive conversions");
    }
    
    return flags;
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
        else if (operand == "iseek" || operand == "skip")
            args.inputSeek = handle_suffixes(value);
        else if (operand == "oseek" || operand == "seek")
            args.outputSeek = handle_suffixes(value);
        else if (operand == "count")
            args.count = handle_suffixes(value);
        else if (operand == "status")
            args.status = value;
        else if (operand == "conv")
            args.conversions = parse_flags(value, true);
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

    return args;
}