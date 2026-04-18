#pragma once

#include <Windows.h>
#include "Arguments.h"

class CopyEngine
{
	bool applyConversions();
	bool applyInputFlags();
	bool applyOutputFlags();
	bool displayStatus(); 

public:
	CopyEngine(Arguments args);
	~CopyEngine();
	void runCopy();

private:
	Arguments args;

	BYTE* buffer = nullptr;
	DWORD bufCapacity;

	std::size_t blocksCopied = 0;
	std::size_t secsElapsed = 0;
	
	HANDLE inputStream = INVALID_HANDLE_VALUE;
	HANDLE outputStream = INVALID_HANDLE_VALUE;
};

