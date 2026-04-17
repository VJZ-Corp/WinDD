#pragma once

#include <string_view>
#include <vector>
#include <string>

namespace util
{
	std::vector<std::string> split(std::string_view str, char delim);
}