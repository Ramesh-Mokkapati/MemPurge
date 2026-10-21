// MemPurge - engine/Optimizer.h
// One "optimize" run over the selected memory areas (SPEC section 3).
// The engine creates no threads and no windows; callers decide where it runs.
#pragma once

#include "engine/AreaResult.h"
#include "engine/RunOptions.h"
#include "engine/RunResult.h"

namespace mp
{
    namespace engine
    {
        // Executes the selected areas in kAreaOrder. Each area is independent; a
        // failure is recorded and the run continues with the next area.
        RunResult RunOptimization(const RunOptions& options);

    } // namespace engine
} // namespace mp
