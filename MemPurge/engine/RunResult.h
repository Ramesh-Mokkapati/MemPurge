// MemPurge - engine/RunResult.h
// Outcome of one optimize run.
#pragma once

#include <cstdint>
#include <vector>

#include "engine/AreaResult.h"

namespace mp
{
    namespace engine
    {
        struct RunResult
        {
            std::vector<AreaResult> results;          // in execution order
            std::uint64_t freedBytes = 0;             // avail-phys after - before, clamped >= 0
            std::uint64_t durationMs = 0;

            bool AllSucceeded() const noexcept;
        };

    } // namespace engine
} // namespace mp
