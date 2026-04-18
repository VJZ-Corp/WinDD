#include "CopyEngine.h"

CopyEngine::CopyEngine(Arguments args)
{
}

CopyEngine::~CopyEngine()
{
	if (inputFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(inputFile);
		inputFile = INVALID_HANDLE_VALUE;
	}

	if (outputFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(outputFile);
		outputFile = INVALID_HANDLE_VALUE;
	}

	if (this->buffer)
	{
		delete[] this->buffer;
		this->buffer = nullptr;
	}

	bufSize = 0;
}
