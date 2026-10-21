// MemPurge - core/SizeFormat.cpp
#include "core/SizeFormat.h"

namespace mp
{
    namespace core
    {
        const ByteUnitNames& DefaultByteUnitNames()
        {
            static const ByteUnitNames names = { { L"B", L"KB", L"MB", L"GB", L"TB" } };
            return names;
        }

        std::wstring FormatBytes(std::uint64_t bytes)
        {
            return FormatBytes(bytes, DefaultByteUnitNames());
        }

        std::wstring FormatBytes(std::uint64_t bytes, const ByteUnitNames& units)
        {
            if (bytes < 1024)
            {
                return std::to_wstring(bytes) + L" " + units[0];
            }

            std::size_t unit = 1;
            std::uint64_t unitSize = 1024;
            while (unit < units.size() - 1 && bytes / unitSize >= 1024)
            {
                unitSize *= 1024;
                ++unit;
            }

            // Rounded tenths of the unit, computed without overflow.
            const std::uint64_t whole = bytes / unitSize;
            const std::uint64_t rem = bytes % unitSize;
            std::uint64_t tenths = whole * 10 + (rem * 10 + unitSize / 2) / unitSize;

            if (tenths >= 10240 && unit < units.size() - 1)
            {
                // e.g. 1048575 B -> 1024.0 KB after rounding -> show as 1.0 MB
                ++unit;
                unitSize *= 1024;
                const std::uint64_t w = bytes / unitSize;
                const std::uint64_t r = bytes % unitSize;
                tenths = w * 10 + (r * 10 + unitSize / 2) / unitSize;
            }

            return std::to_wstring(tenths / 10) + L"." + std::to_wstring(tenths % 10) + L" " + units[unit];
        }

    } // namespace core
} // namespace mp
