#include "CopyEngine.h"
#include <iostream>

CopyEngine::CopyEngine(Arguments args)
{
	this->args = args;
	blocksCopied = 0;
	secsElapsed = 0;
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
	if (!performPrechecks())
		return;

	std::size_t blocks_remaining = this->args.count;

	// count = 0 means read until EOF, which effectively makes this check underflow blocks_remaining (a huge #)
	while (!this->args.count || blocks_remaining)
	{
		//if (this->args.count && blocks_remaining == 0)
		//	break; // we are done with file

		DWORD bytes_read;
		BOOL ok = ReadFile(
			inputFile,
			this->buffer,
			this->args.inputBlockSize,
			&bytes_read,
			nullptr
		);

		if (!ok || !bytes_read)
			break; // something went wrong with reading

		DWORD total_written = 0;
		while (total_written < bytes_read)
		{
			DWORD bytes_written;
			ok = WriteFile(
				outputFile,
				this->buffer + total_written,
				bytes_read - total_written,
				&bytes_written,
				nullptr
			);

			if (!ok)
				return; // stop engine to prevent corruption

			total_written += bytes_written;
		}

		blocksCopied++;
		if (this->args.count)
			blocks_remaining--;
	}
}

bool CopyEngine::performPrechecks()
{
	// request maximum of either IBS or OBS for safety
	if (!allocBuffer(max(static_cast<DWORD>(this->args.inputBlockSize), static_cast<DWORD>(this->args.outputBlockSize))))
		return false;

	if (!open(this->args.inputFilename.c_str()))
		return false; // input file cannot be opened

	if (!open(this->args.outputFilename.c_str(), false))
		return false; // output file cannot be opened

	return true;
}

bool CopyEngine::open(LPCSTR path, bool is_read, BOOL truncate)
{
	if (is_read && *path == '\0') // path is empty, assume stdin
	{
		inputFile = GetStdHandle(STD_INPUT_HANDLE);
		return inputFile != INVALID_HANDLE_VALUE;
	}

	if (*path == '\0') // path is empty AND not in read mode, assume stdout
	{
		outputFile = GetStdHandle(STD_OUTPUT_HANDLE);
		return outputFile != INVALID_HANDLE_VALUE;
	}

	if (is_read)
	{
		inputFile = CreateFileA(
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

	outputFile = CreateFileA(
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

bool CopyEngine::allocBuffer(DWORD size)
{
	this->buffer = new (std::nothrow) BYTE[size];
	bufSize = this->buffer ? size : 0;
	return this->buffer != nullptr;
}
