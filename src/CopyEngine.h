#pragma once

#include <Windows.h>
#include <chrono>
#include "Arguments.h"

class CopyEngine
{
	bool applyConversions();
	bool applyInputFlags();
	bool applyOutputFlags();
	void displayStatus(bool ongoing) const; 

public:
	CopyEngine(Arguments args);
	~CopyEngine();
	void runCopy();

private:
	Arguments args;
	BYTE* buffer = nullptr;
	DWORD bufCapacity;
	
	HANDLE inputFile = INVALID_HANDLE_VALUE;
	HANDLE outputFile = INVALID_HANDLE_VALUE;

	std::size_t bytesCopied = 0;
	std::chrono::steady_clock::time_point startTime;

	std::size_t wholeRecordsIn = 0;
	std::size_t wholeRecordsOut = 0;
	std::size_t partialRecordsIn = 0;
	std::size_t partialRecordsOut = 0;
};

