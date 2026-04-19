#include "CopyEngine.h"
#include <iostream>
#include "WinIO.h"

CopyEngine::CopyEngine(Arguments args)
{
	this->args = args;

	inputFile = WinIO::open(this->args.inputFilename.c_str());
	outputFile = WinIO::open(this->args.outputFilename.c_str(), FALSE);

	// buffer capacity should be max(ibs, obs) for safety
	bufCapacity = static_cast<DWORD>(max(this->args.inputBlockSize, this->args.outputBlockSize));
	this->buffer = new (std::nothrow) BYTE[bufCapacity];
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
}

void CopyEngine::runCopy()
{
	/* PERFORM PRECHECKS */
	if (this->buffer == nullptr)
	{
		std::cerr << "buf null\n";
		return;
	}

	if (inputFile == INVALID_HANDLE_VALUE)
	{
		std::cerr << "dd: failed to open '" << this->args.inputFilename << "': "; 
		WinIO::printError();
		std::cerr << "\n";
		return;
	}

	if (outputFile == INVALID_HANDLE_VALUE)
	{
		std::cerr << "dd: failed to open '" << this->args.outputFilename << "': ";
		WinIO::printError();
		std::cerr << "\n";
		return;
	}

	// keep track how many bytes from the start of buffer contains meaningful but unwritten data
	std::size_t meaningful = 0;
	bool end_of_file = false;
	std::size_t blocks_read = 0;

	// exit if end of file is reached or count is not 0 but the # of blocks read exceeded demand
	while (!end_of_file)
	{
		/* READ PHASE */
		if (!this->args.count || blocks_read < this->args.count)
		{
			// if buffer becomes full at any point, block reading until buffer is uncongested
			DWORD bytes_to_read = static_cast<DWORD>(min(this->args.inputBlockSize, bufCapacity - meaningful));
			DWORD bytes_actually_read = 0;

			// write starting at meaningful point
			if (!ReadFile(inputFile, this->buffer + meaningful, bytes_to_read, &bytes_actually_read, nullptr))
			{
				std::cerr << "read failed\n";
				return;
			}

			if (bytes_actually_read == 0)
				end_of_file = true;
			else
			{
				meaningful += bytes_actually_read; // move meaningful boundary
				blocks_read++;
			}
		}

		// check whether if there is enough data to write one 'obs'-sized block
		while (meaningful >= this->args.outputBlockSize)
		{
			if (WinIO::write(outputFile, this->buffer, this->args.outputBlockSize))
			{
				// move meaningful boundary back by OBS bytes
				meaningful -= this->args.outputBlockSize;
				blocksCopied++;

				// minimal buffer compaction
				if (meaningful > 0)
					std::memmove(this->buffer, this->buffer + this->args.outputBlockSize, meaningful);
			}
			else
			{
				std::cerr << "Write failed\n";
				return;
			}
		}
	}

	// flush one last time if there is still meaningful data leftover
	if (meaningful > 0)
		if (!WinIO::write(outputFile, this->buffer, meaningful))
			std::cerr << "Final flush failed!\n";
}