#include "util.h"

namespace util
{
    static std::vector<std::string> split(std::string_view str, char delim)
    {
        std::vector<std::string> out;
        size_t start = 0;

        while (start <= str.size())
        {
            size_t end = str.find(delim, start);

            if (end == std::string_view::npos)
            {
                out.emplace_back(str.substr(start));
                break;
            }

            out.emplace_back(str.substr(start, end - start));
            start = end + 1;
        }

        return out;
    }
}