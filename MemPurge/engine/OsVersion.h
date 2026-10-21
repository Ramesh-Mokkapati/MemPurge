// MemPurge - engine/OsVersion.h
// Real OS version via ntdll!RtlGetVersion (not subject to manifest shimming).
#pragma once

#include <cstdint>

namespace mp
{
    namespace engine
    {
        struct OsVersion
        {
            std::uint32_t major = 0;
            std::uint32_t minor = 0;
            std::uint32_t build = 0;
        };

        OsVersion GetOsVersion() noexcept;
        bool IsWindows10OrLater() noexcept;

    } // namespace engine
} // namespace mp
