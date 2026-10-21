// MemPurge - core/CommandLineOptions.cpp
#include "core/CommandLineOptions.h"

namespace mp
{
    namespace core
    {
        bool CommandLineOptions::HasError() const noexcept
        {
            return !unknownSwitches.empty();
        }

        bool CommandLineOptions::IsHeadless() const noexcept
        {
            return areas != 0;
        }

    } // namespace core
} // namespace mp
