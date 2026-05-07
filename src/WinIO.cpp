#include "WinIO.h"
#include <iostream>

HANDLE WinIO::open(const LPCSTR path, const BOOL is_reading, const BOOL truncate, const ULONGLONG offset)
{
    if (is_reading && *path == '\0') // path is empty (default), assume stdin
        return GetStdHandle(STD_INPUT_HANDLE);

    if (*path == '\0') // path is empty AND not in read mode, assume stdout
        return GetStdHandle(STD_OUTPUT_HANDLE);

    HANDLE file = CreateFileA(
        path,
        is_reading ? GENERIC_READ : GENERIC_WRITE,
        is_reading ? FILE_SHARE_READ : NULL,
        nullptr,
        truncate ? CREATE_ALWAYS : OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        nullptr
    );
    
    if (file == INVALID_HANDLE_VALUE)
        return file;

    LARGE_INTEGER large_int;
    large_int.QuadPart = offset;
    SetFilePointerEx(file, large_int, nullptr, FILE_BEGIN);
    return file;
}

BOOL WinIO::write(const HANDLE file, const BYTE* data, const DWORD amount_bytes_to_write)
{
    DWORD total_written = 0;
    while (total_written < amount_bytes_to_write)
    {
        DWORD amount_left_to_write = amount_bytes_to_write - total_written;
        DWORD bytes_actually_written = 0;

        if (!WriteFile(file, data + total_written, amount_left_to_write, &bytes_actually_written, nullptr))
            return FALSE;

        if (!bytes_actually_written)
            return FALSE; // something went wrong, bytes were not written at all

        total_written += bytes_actually_written;
    }

    return TRUE;
}

LONGLONG WinIO::getPhysicalDiskSize(HANDLE device)
{
    GET_LENGTH_INFORMATION info{};
    BOOL ok = DeviceIoControl(
        device,
        IOCTL_DISK_GET_LENGTH_INFO,
        nullptr,
        0,
        &info,
        sizeof(info),
        nullptr,
        nullptr
    );

    if (ok)
        return info.Length.QuadPart;
    else
        return -1;
}

void WinIO::printError()
{
    DWORD err = GetLastError();
    if (err == S_OK)
    {
        // most likely wrong file
        std::cerr << "No such file or directory";
        return;
    }

    LPSTR msg = nullptr;
    DWORD len = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        err,
        0, // default language
        (LPSTR)&msg, // WinAPI is picky: have to use C-style raw casting
        0,
        nullptr
    );

    if (len && msg)
        std::cerr << msg;
    else
        std::cerr << "Code " << err << " (unable to format message)";

    LocalFree(msg);
}