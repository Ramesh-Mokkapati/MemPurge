// MemPurge - engine/RunResult.cpp
#include "engine/RunResult.h"

namespace mp
{
    namespace engine
    {
        bool RunResult::AllSucceeded() const noexcept
        {
            for (const auto& r : results)
            {
                if (r.status != AreaStatus::Success)
                {
                    return false;
                }
            }

            return true;
        }

    } // namespace engine
} // namespace mp
