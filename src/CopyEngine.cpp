#include "CopyEngine.h"
#include "WinIO.h"
#include "Status.h"
#include <iostream>
#include <thread>

CopyEngine::CopyEngine(Arguments args)
{
	this->args = args;

	inputFile = WinIO::open(args, TRUE);
	outputFile = WinIO::open(args, FALSE);
	
	// buffer capacity should be max(ibs, obs) for safety
	bufCapacity = static_cast<DWORD>(max(args.inputBlockSize, args.outputBlockSize));
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
		std::cerr << "\nHint: are you admin?\nHint: the file may not exist";
		return false;
	}

	if (outputFile == INVALID_HANDLE_VALUE)
	{
		std::cerr << "dd: failed to open '" << this->args.outputFilename << "': ";
		WinIO::printError();

		if (IS_SET(this->args.conversions, Conversion::EXCL))
			std::cerr << "\nHint: you have the excl conversion flag set";
		if (IS_SET(this->args.conversions, Conversion::NOCREAT))
			std::cerr << "\nHint: you have the nocreat conversion flag set";

		std::cerr << "\nHint: are you admin?\n";
		return false;
	}

	return true;
}

inline bool CopyEngine::writeBlock(std::size_t block_sz)
{
	if (!WinIO::write(outputFile, this->buffer, block_sz))
	{
		inProgressCopying = false;
		permissionToPrintError.wait();

		std::cerr << "\r\033[2Kdd: failed to write to '" << this->args.outputFilename << "': ";
		WinIO::printError();

		return false;
	}

	return true;
}

void CopyEngine::runCopyJob()
{
	if (!performPrechecks())
	{
		// still allow monitoring to start but it will find that isStillCopying is false and terminate
		permissionToStartMonitoring.count_down(); 
		return;
	}

	// physical drives do not notify EOF, need to check disk size
	long long disk_size = MAXLONGLONG;
	if (args.inputFilename.find("\\\\.\\PhysicalDrive") == 0)
		disk_size = WinIO::getPhysicalDiskSize(inputFile);

	if (disk_size < 0)
	{
		std::cerr << "\r\033[2Kdd: failed to get size of '" << this->args.inputFilename << "': ";
		WinIO::printError();
		return;
	}

	// keep track how many bytes from the start of buffer contains meaningful but unwritten data
	std::size_t meaningful = 0;

	inProgressCopying = true;
	permissionToStartMonitoring.count_down(); // allow monitoring thread to proceed

	// begin stopwatch
	startTime = std::chrono::steady_clock::now(); 

	// exit if end of file is reached 
	bool end_of_file = false;
	while (!end_of_file)
	{
		// READ PHASE: count is not 0 but the # of blocks read exceeded demand
		if (!this->args.count || bytesCopied <= this->args.count * this->args.inputBlockSize)
		{
			// if buffer becomes full at any point OR disk size is reached, block reading until buffer is uncongested
			DWORD bytes_to_read = min(min(this->args.inputBlockSize, bufCapacity - meaningful), disk_size - bytesCopied);
			DWORD bytes_actually_read = 0;

			// read starting at meaningful point
			if (!ReadFile(inputFile, this->buffer + meaningful, bytes_to_read, &bytes_actually_read, nullptr)
					&& !IS_SET(this->args.conversions, Conversion::NOERR))
			{
				inProgressCopying = false;
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
				BYTE* block = this->buffer + meaningful;

				// sync (pad with zeroes)
				if (IS_SET(this->args.conversions, Conversion::SYNC) && bytes_actually_read < this->args.inputBlockSize)
				{
					std::memset(block + bytes_actually_read, 0, this->args.inputBlockSize - bytes_actually_read);
					bytes_actually_read = this->args.inputBlockSize;
				}

				// swab (swap byte pairs)
				if (IS_SET(this->args.conversions, Conversion::SWAB))
					for (DWORD i = 0; i + 1 < bytes_actually_read; i += 2)
						std::swap(block[i], block[i + 1]);

				// ucase (turns every ASCII character uppercase)
				if (IS_SET(this->args.conversions, Conversion::UCASE))
					for (DWORD i = 0; i < bytes_actually_read; i++)
						block[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(block[i])));

				// lcase (turns every ASCII character lowercase)
				if (IS_SET(this->args.conversions, Conversion::LCASE))
					for (DWORD i = 0; i < bytes_actually_read; i++)
						block[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(block[i])));

				if (bytes_actually_read < this->args.inputBlockSize)
					partialRecordsIn++;
				else
					wholeRecordsIn++;

				// move meaningful boundary
				meaningful += bytes_actually_read;
			}
		}
		else
			end_of_file = true; // done reading: finish writing and exit loop

		/* READ PHASE DONE, WRITE PHASE BELOW */

		// check whether if there is enough data to write one 'obs'-sized block
		while (meaningful >= this->args.outputBlockSize)
		{
			bool all_zeros = false;

			// sparse (skip blocks of zeroes)
			if (IS_SET(this->args.conversions, Conversion::SPARSE))
			{
				all_zeros = true;
				const BYTE* ptr = this->buffer;
				const BYTE* end = ptr + this->args.outputBlockSize;

				while (ptr < end)
				{
					if (*ptr++ != 0) // zero chain broken
					{
						all_zeros = false;
						break;
					}
				}
			}

			if (!all_zeros)
			{
				if (!writeBlock(this->args.outputBlockSize))
					return;

				bytesCopied += this->args.outputBlockSize;
				wholeRecordsOut++;
			}

			// move meaningful boundary back by OBS bytes
			meaningful -= this->args.outputBlockSize;

			// minimal buffer compaction
			if (meaningful > 0)
				std::memmove(this->buffer, this->buffer + this->args.outputBlockSize, meaningful);
		}
	}

	// flush one last time if there is still meaningful data leftover
	if (meaningful > 0)
	{
		if (IS_SET(this->args.conversions, Conversion::SYNC))
		{
			std::memset(this->buffer + meaningful, 0, this->args.outputBlockSize - meaningful);
			meaningful = this->args.outputBlockSize;
		}

		if (!writeBlock(meaningful))
			return;

		bytesCopied += meaningful;

		if (meaningful == this->args.outputBlockSize)
			wholeRecordsOut++;
		else
			partialRecordsOut++;
	}

	inProgressCopying = false;

	// display results
	Status::displayRecordsSummary(wholeRecordsIn, wholeRecordsOut, 
									partialRecordsIn, partialRecordsOut, 
									this->args.status == "progress");
	if (this->args.status != "noxfer")
		Status::displayXferStats(inProgressCopying, startTime, bytesCopied);
}

void CopyEngine::monitorStatus()
{
	// block checking until copying starts
	permissionToStartMonitoring.wait();

	if (this->args.status == "progress")
	{
		while (inProgressCopying)
		{
			Status::displayXferStats(inProgressCopying, startTime, bytesCopied.load());
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::cout << "\r\033[2K"; // move cursor to beginning and wipe line if status printing ongoing
		}
	}

	// loop exited, can handle printing error
	permissionToPrintError.count_down();
}