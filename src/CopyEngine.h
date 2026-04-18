#pragma once

#include <Windows.h>
#include "Arguments.h"

class CopyEngine
{
	bool validateArgs();
	bool applyConversions();
	bool applyInputFlags();
	bool applyOutputFlags();

	bool open(LPCWSTR path, BOOL truncate, bool isRead);
	bool allocBuffer(DWORD size);
	bool displayStatus(); 

public:
	CopyEngine(Arguments args);
	~CopyEngine();
	void run();

private:
	std::size_t blocksCopied;
	std::size_t secsElapsed;
	
	HANDLE inputFile = INVALID_HANDLE_VALUE;
	HANDLE outputFile = INVALID_HANDLE_VALUE;

	BYTE* buffer = nullptr;
	DWORD bufSize = 0;
};

