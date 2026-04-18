#pragma once

#include <unordered_map>
#include <string>
#include "Arguments.h"

class ArgParser
{
public:
	ArgParser(int argc, char* argv[]);
	Arguments parse() const;

private:
	std::unordered_map<std::string, std::string> operandMap;
};

