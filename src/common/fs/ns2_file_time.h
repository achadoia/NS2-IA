// SPDX-License-Identifier: GPL-3.0-or-later
// NS2-IA: a file's last write time as the number the official (MSVC) Fritchii builds store in
// their cache stamps: Windows FILETIME ticks (100 ns since 1601). MinGW's libstdc++ counts
// std::filesystem times from another epoch, in whole seconds, so a MinGW build would never match
// the stamps a Fritchii build wrote and would prepare all of its shaders again.

#pragma once

#include <cstdint>
#include <filesystem>
#include <system_error>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace Common::FS {

inline std::int64_t WriteTimeTicks(const std::filesystem::path& path, std::error_code& error) {
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA data{};
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &data)) {
        error = std::error_code(static_cast<int>(GetLastError()), std::system_category());
        return 0;
    }
    error.clear();
    return static_cast<std::int64_t>(
        (static_cast<std::uint64_t>(data.ftLastWriteTime.dwHighDateTime) << 32) |
        data.ftLastWriteTime.dwLowDateTime);
#else
    const auto time = std::filesystem::last_write_time(path, error);
    return error ? 0 : static_cast<std::int64_t>(time.time_since_epoch().count());
#endif
}

} // namespace Common::FS
