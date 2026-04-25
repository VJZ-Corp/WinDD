#include "CopyEngine.h"
#include <iostream>
#include <thread>
#include "WinIO.h"
#include "Status.h"

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

bool CopyEngine::performPrechecks()
{
	if (this->buffer == nullptr)
	{
		std::cerr << "dd: failed to allocate temporary buffer\n";
		return false;
	}

	if (inputFile == INVALID_HANDLE_VALUE)
	{
		std::cerr << "dd: failed to open '" << this->args.inputFilename << "': ";
		WinIO::printError();
		std::cerr << "\n";
		return false;
	}

	if (outputFile == INVALID_HANDLE_VALUE)
	{
		std::cerr << "dd: failed to open '" << this->args.outputFilename << "': ";
		WinIO::printError();
		std::cerr << "\n";
		return false;
	}

	return true;
}

void CopyEngine::runCopyJob()
{
	if (!performPrechecks())
	{
		// still allow monitoring to start but it will find that isStillCopying is false and die
		permissionToStart.count_down(); 
		return;
	}

	isStillCopying = true;
	permissionToStart.count_down(); // allow monitoring thread to proceed

	// keep track how many bytes from the start of buffer contains meaningful but unwritten data
	std::size_t meaningful = 0;
	bool end_of_file = false;
	
	startTime = std::chrono::steady_clock::now(); // begin stopwatch

	// exit if end of file is reached or count is not 0 but the # of blocks read exceeded demand
	while (!end_of_file)
	{
		/* READ PHASE */
		if (!this->args.count || wholeRecordsIn + partialRecordsIn < this->args.count)
		{
			// if buffer becomes full at any point, block reading until buffer is uncongested
			DWORD bytes_to_read = static_cast<DWORD>(min(this->args.inputBlockSize, bufCapacity - meaningful));
			DWORD bytes_actually_read = 0;

			// write starting at meaningful point
			if (!ReadFile(inputFile, this->buffer + meaningful, bytes_to_read, &bytes_actually_read, nullptr))
			{
				isStillCopying = false;
				permissionToPrintError.wait();

				std::cerr << "\r\033[2Kdd: failed to read from '" << this->args.inputFilename << "': ";
				WinIO::printError();
				std::cerr << "Hint: Block size should be a power of 2 and greater than or equal to 512 when reading volumes or physical drives.\n";
				return;
			}

			if (bytes_actually_read == 0)
				end_of_file = true;
			else
			{
				meaningful += bytes_actually_read; // move meaningful boundary

				if (bytes_actually_read < this->args.inputBlockSize)
					partialRecordsIn++;
				else
					wholeRecordsIn++;
			}
		}
		else
			end_of_file = true; // done reading: finish writing and exit loop

		// check whether if there is enough data to write one 'obs'-sized block
		while (meaningful >= this->args.outputBlockSize)
		{
			if (WinIO::write(outputFile, this->buffer, this->args.outputBlockSize))
			{
				// move meaningful boundary back by OBS bytes
				meaningful -= this->args.outputBlockSize;
				bytesCopied += this->args.outputBlockSize;
				wholeRecordsOut++;

				// minimal buffer compaction
				if (meaningful > 0)
					std::memmove(this->buffer, this->buffer + this->args.outputBlockSize, meaningful);
			}
			else
			{
				isStillCopying = false;
				permissionToPrintError.wait();

				std::cerr << "\r\033[2Kdd: failed to write to '" << this->args.outputFilename << "': ";
				WinIO::printError();
				std::cerr << "\n";
				return;
			}
		}
	}

	// flush one last time if there is still meaningful data leftover
	if (meaningful > 0)
	{
		if (!WinIO::write(outputFile, this->buffer, meaningful))
		{
			isStillCopying = false;
			permissionToPrintError.wait();

			std::cerr << "\r\033[2Kdd: failed to write to '" << this->args.outputFilename << "': ";
			WinIO::printError();
			std::cerr << "\n";
			return;
		}

		partialRecordsOut++;
		bytesCopied += meaningful;
	}

	isStillCopying = false;
	Status::displayRecordsSummary(wholeRecordsIn, wholeRecordsOut, partialRecordsIn, partialRecordsOut);

	if (this->args.status != "noxfer")
		Status::displayXferStats(isStillCopying, startTime, bytesCopied);
}

void CopyEngine::monitorStatus()
{
	// block checking until copying starts
	permissionToStart.wait();

	if (this->args.status == "progress")
	{
		while (isStillCopying)
		{
			Status::displayXferStats(isStillCopying, startTime, bytesCopied.load());
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	// loop exited, can handle printing error
	permissionToPrintError.count_down();
}