// MemPurge - core/SizeFormat.h
#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace mp
{
    namespace core
    {
        // Unit labels B, KB, MB, GB, TB (index 0..4). The UI passes localized labels
        // loaded from the STRINGTABLE; the default overload uses the English labels.
        using ByteUnitNames = std::array<std::wstring, 5>;

        const ByteUnitNames& DefaultByteUnitNames();

        // Binary units (1 KB = 1024 B). Bytes are shown without decimals ("512 B"),
        // larger units with one decimal ("1.5 KB"). A value that rounds to 1024.0 of
        // a unit is promoted to the next unit ("1.0 MB" instead of "1024.0 KB").
        std::wstring FormatBytes(std::uint64_t bytes);
        std::wstring FormatBytes(std::uint64_t bytes, const ByteUnitNames& units);

    } // namespace core
} // namespace mp
