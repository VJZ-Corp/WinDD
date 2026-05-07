#pragma once

#include <Windows.h>

namespace WinIO
{
	HANDLE open(const LPCSTR path, const ULONGLONG offset, const BOOL is_reading = true, const BOOL truncate = FALSE);
	BOOL write(const HANDLE file, const BYTE* data, const DWORD amount_bytes_to_write);
	LONGLONG getPhysicalDiskSize(HANDLE device);
	void printError();
}