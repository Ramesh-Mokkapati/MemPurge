// MemPurge - engine/RunOptions.h
// Input of one optimize run. Plain data.
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "engine/Areas.h"

namespace mp
{
    namespace engine
    {
        // index is zero-based; called before each area starts.
        typedef std::function<void(std::size_t index, std::size_t count, Area area)> ProgressCallback;

        struct RunOptions
        {
            std::uint32_t areas = 0;                  // Area bitmask; normalized by the run
            std::vector<std::wstring> exclusions;     // process names (normalized by the run)
            bool lowPriority = false;
            ProgressCallback progress;
        };

    } // namespace engine
} // namespace mp
