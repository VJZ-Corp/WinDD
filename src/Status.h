#pragma once

#include <chrono>

namespace Status
{
	void displayRecordsSummary(std::size_t wri, std::size_t wro, std::size_t pri, std::size_t pro);
	void displayXferStats(bool ongoing, std::chrono::steady_clock::time_point start, std::size_t bytes_copied);
}