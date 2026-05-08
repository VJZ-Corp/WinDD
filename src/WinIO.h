#pragma once

#include "Arguments.h"
#include <Windows.h>

namespace WinIO
{
	HANDLE open(const Arguments& args, const BOOL is_reading);
	BOOL write(const HANDLE file, const BYTE* data, const DWORD amount_bytes_to_write);
	LONGLONG getPhysicalDiskSize(HANDLE device);
	void printError();
}