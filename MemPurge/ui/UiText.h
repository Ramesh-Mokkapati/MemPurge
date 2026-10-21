// MemPurge - ui/UiText.h
// Localizable text helpers: everything user-visible comes from the STRINGTABLE.
#pragma once

#include <cstdint>

#include "OptimizeReason.h"
#include "engine/Optimizer.h"

namespace mp
{
    namespace ui
    {
        CString LoadStr(UINT id);
        CString FormatBytesUi(std::uint64_t bytes);
        CString AreaDisplayName(engine::Area area);
        CString StatusText(engine::AreaStatus status);
        CString ReasonText(OptimizeReason reason);

        // "<Area>: <Status>" or "<Area>: <Status> (error N: message)".
        CString FormatResultLine(const engine::AreaResult& result);

        // Multi-line text for the Details message box (OS version + one line per area).
        CString FormatDetails(const engine::RunResult& result);

    } // namespace ui
} // namespace mp
