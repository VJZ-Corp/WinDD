#pragma once

#include <Windows.h>
#include "Arguments.h"

class CopyEngine
{
	bool performPrechecks();
	bool applyConversions();
	bool applyInputFlags();
	bool applyOutputFlags();
	bool open(LPCSTR path, bool is_read = true, BOOL truncate = TRUE);

	bool allocBuffer(DWORD size);
	bool displayStatus(); 

public:
	CopyEngine(Arguments args);
	~CopyEngine();
	void run();

private:
	Arguments args;

	std::size_t blocksCopied;
	std::size_t secsElapsed;
	
	HANDLE inputFile = INVALID_HANDLE_VALUE;
	HANDLE outputFile = INVALID_HANDLE_VALUE;

	BYTE* buffer = nullptr;
	DWORD bufSize = 0;
};

