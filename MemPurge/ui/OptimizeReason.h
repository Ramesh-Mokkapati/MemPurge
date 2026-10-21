// MemPurge - ui/OptimizeReason.h
// What triggered an optimization (shown in the notification).
#pragma once

namespace mp
{
    namespace ui
    {
        enum class OptimizeReason
        {
            Manual,
            Hotkey,
            Interval,
            LowMemory,
            Tray
        };

    } // namespace ui
} // namespace mp
