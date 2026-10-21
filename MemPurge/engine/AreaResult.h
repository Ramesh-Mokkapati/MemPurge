// MemPurge - engine/AreaResult.h
// Result of one memory area in a run (SPEC section 3.2). Plain data.
#pragma once

#include <cstdint>
#include <string>

#include "engine/Areas.h"
#include "engine/Status.h"

namespace mp
{
    namespace engine
    {
        struct AreaResult
        {
            Area area = Area::WorkingSet;
            AreaStatus status = AreaStatus::Failed;
            unsigned long win32Error = 0;   // 0 on success
            std::wstring message;           // system message text; empty on success
            std::uint64_t durationMs = 0;
        };

    } // namespace engine
} // namespace mp
