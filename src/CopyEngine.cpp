#include "CopyEngine.h"
#include <iostream>
#include <iomanip>
#include <cmath>
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
		std::cerr << "dd: failed to allocate temporary buffer\n";
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
	startTime = std::chrono::steady_clock::now(); // begin stopwatch

	// exit if end of file is reached or count is not 0 but the # of blocks read exceeded demand
	while (!end_of_file)
	{
		/* READ PHASE */
		if (!this->args.count || wholeRecordsIn < this->args.count)
		{
			// if buffer becomes full at any point, block reading until buffer is uncongested
			DWORD bytes_to_read = static_cast<DWORD>(min(this->args.inputBlockSize, bufCapacity - meaningful));
			DWORD bytes_actually_read = 0;

			// write starting at meaningful point
			if (!ReadFile(inputFile, this->buffer + meaningful, bytes_to_read, &bytes_actually_read, nullptr))
			{
				std::cerr << "dd: failed to read from '" << this->args.inputFilename << "'\n";
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
				std::cerr << "dd: failed to write to '" << this->args.outputFilename << "': ";
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
			std::cerr << "dd: failed to write to '" << this->args.outputFilename << "': ";
			WinIO::printError();
			std::cerr << "\n";
			return;
		}

		partialRecordsOut++;
	}

	displayStatus(false);
}

void CopyEngine::displayStatus(bool ongoing) const
{
	using namespace std::chrono;

	double elapsed_secs = duration<double>(steady_clock::now() - startTime).count();

	if (!ongoing)
	{
		std::cout << wholeRecordsIn << "+" << partialRecordsIn << " records in\n"
			<< wholeRecordsOut << "+" << partialRecordsOut << " records out\n";
	}
		
	double kb = bytesCopied / 1000.0;
	double kib = bytesCopied / 1024.0;
	double mb = kb / 1000.0;
	double mib = kib / 1024.0;

	std::cout << bytesCopied << " bytes";

	if (bytesCopied >= 1024)
	{
		std::cout << " (";

		// pick best decimal unit
		if (mb >= 1.0)
			std::cout << std::fixed << std::setprecision(1) << mb << " MB";
		else
			std::cout << std::fixed << std::setprecision(1) << kb << " KB";

		std::cout << ", ";

		// binary unit
		if (mib >= 1.0)
			std::cout << std::fixed << std::setprecision(1) << mib << " MiB";
		else
			std::cout << std::fixed << std::setprecision(1) << kib << " KiB";

		std::cout << ")";
	}

	double rate = (elapsed_secs > 0.0) ? (bytesCopied / elapsed_secs) : 0.0;
	const char* unit = "kB/s";
	rate /= 1024.0; // GNU's dd apparently uses kB in both a binary and decimal context

	if (rate >= 1024.0) 
	{
		rate /= 1024.0; 
		unit = "MB/s"; 
	}

	if (rate >= 1024.0) 
	{ 
		rate /= 1024.0; 
		unit = "GB/s"; 
	}

	std::cout << " copied , " << elapsed_secs << " s, " << rate << " " << unit << '\n';
}