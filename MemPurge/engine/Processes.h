// MemPurge - engine/Processes.h
#pragma once

#include <string>
#include <vector>

namespace mp
{
    namespace engine
    {
        // Normalized (see NormalizeProcessName), deduplicated, sorted image names of the
        // running processes (Toolhelp32 snapshot). Read-only; performs no memory operation.
        std::vector<std::wstring> EnumerateProcessNames();

    } // namespace engine
} // namespace mp
