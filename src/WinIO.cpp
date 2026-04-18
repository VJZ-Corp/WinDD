#include "WinIO.h"

HANDLE WinIO::open(const LPCSTR path, const BOOL is_reading, const BOOL truncate)
{
    if (is_reading && *path == '\0') // path is empty (default), assume stdin
        return GetStdHandle(STD_INPUT_HANDLE);

    if (*path == '\0') // path is empty AND not in read mode, assume stdout
        return GetStdHandle(STD_OUTPUT_HANDLE);

    if (is_reading)
    {
        return CreateFileA(
            path,
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
            nullptr
        );
    }

    return CreateFileA(
        path,
        GENERIC_WRITE,
        NULL,
        nullptr,
        truncate ? CREATE_ALWAYS : OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        nullptr
    );
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
