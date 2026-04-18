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

void CopyEngine::run()
{
}

bool CopyEngine::open(LPCWSTR path, BOOL truncate, bool isRead)
{
	if (isRead)
	{
		inputFile = CreateFileW(
			path,
			GENERIC_READ,
			FILE_SHARE_READ,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
			nullptr
		);
		
		return inputFile != INVALID_HANDLE_VALUE;
	}

	outputFile = CreateFileW(
		path,
		GENERIC_WRITE,
		0,
		nullptr,
		truncate ? CREATE_ALWAYS : OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		nullptr
	);

	return outputFile != INVALID_HANDLE_VALUE;
}
