#pragma once

#include <Windows.h>
#include <chrono>
#include <latch>
#include "Arguments.h"

class CopyEngine
{
	bool performPrechecks();

public:
	CopyEngine(Arguments args);
	~CopyEngine();
	void runCopyJob();
	void monitorStatus();

private:
	Arguments args;
	std::latch permissionToStart{ 1 };
	std::latch permissionToPrintError{ 1 };
	std::atomic<bool> isStillCopying{ false };

	BYTE* buffer = nullptr;
	DWORD bufCapacity;
	
	HANDLE inputFile = INVALID_HANDLE_VALUE;
	HANDLE outputFile = INVALID_HANDLE_VALUE;

	std::atomic<std::size_t> bytesCopied = 0;
	std::chrono::steady_clock::time_point startTime;

	std::size_t wholeRecordsIn = 0;
	std::size_t wholeRecordsOut = 0;
	std::size_t partialRecordsIn = 0;
	std::size_t partialRecordsOut = 0;
};

