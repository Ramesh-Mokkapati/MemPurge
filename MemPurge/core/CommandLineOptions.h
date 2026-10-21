// MemPurge - core/CommandLineOptions.h
// Parsed command line (SPEC section 4).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mp
{
    namespace core
    {
        struct CommandLineOptions
        {
            std::uint32_t areas = 0;                    // normalized Area bitmask
            bool help = false;                          // /? or /Help
            bool minimized = false;                     // /Minimized
            std::vector<std::wstring> unknownSwitches;  // anything not recognized

            bool HasError() const noexcept;
            // Any area switch present -> headless run.
            bool IsHeadless() const noexcept;
        };

    } // namespace core
} // namespace mp
