#include "Status.h"
#include <iostream>

void Status::displayRecordsSummary(std::size_t wri, std::size_t wro, std::size_t pri, std::size_t pro)
{
	std::cout << "\r\033[2K"; // overwrite previous line
	std::cout << wri << "+" << pri << " records in\n"
			  << wro << "+" << pro << " records out\n";
}

void Status::displayXferStats(bool ongoing, std::chrono::steady_clock::time_point start, std::size_t bytes_copied)
{
	double elapsed_secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

	if (ongoing)
		std::cout << "\r\033[2K"; // move cursor to beginning and wipe line if status printing ongoing

	std::cout << bytes_copied << " bytes";

	double kb = bytes_copied / 1000.0;
	double kib = bytes_copied / 1024.0;

	if (bytes_copied >= 1024)
	{
		double mb = kb / 1000.0;
		double mib = kib / 1024.0;
		double gb = mb / 1000.0;
		double gib = mib / 1024.0;

		std::cout << " (";

		// pick best decimal unit
		if (gb >= 1.0)
			std::cout << std::fixed << std::setprecision(1) << gb << " GB";
		else if (mb >= 1.0)
			std::cout << std::fixed << std::setprecision(1) << mb << " MB";
		else
			std::cout << std::fixed << std::setprecision(1) << kb << " KB";

		std::cout << ", ";

		// binary unit
		if (gib >= 1.0)
			std::cout << std::fixed << std::setprecision(1) << mb << " GiB";
		else if (mib >= 1.0)
			std::cout << std::fixed << std::setprecision(1) << mib << " MiB";
		else
			std::cout << std::fixed << std::setprecision(1) << kib << " KiB";

		std::cout << ")";
	}

	double rate = (elapsed_secs > 0.0) ? (bytes_copied / elapsed_secs) : 0.0;
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

	std::cout << " copied, " << std::fixed;

	if (ongoing)
		std::cout << std::setprecision(0); // whole number when printing status
	else
		std::cout << std::setprecision(6);

	std::cout << elapsed_secs << " s, " << std::setprecision(1) << rate << " " << unit;

	if (ongoing)
		std::cout << std::flush;
	else
		std::cout << '\n';
}
