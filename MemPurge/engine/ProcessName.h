// MemPurge - engine/ProcessName.h
// Process image-name normalization shared by the engine (exclusion matching)
// and core (settings). Pure helpers.
#pragma once

#include <string>
#include <vector>

namespace mp
{
    namespace engine
    {
        // Trims whitespace, removes any directory part, lower-cases (invariant) and
        // strips a trailing ".exe".  "Chrome.EXE " -> "chrome".
        std::wstring NormalizeProcessName(const std::wstring& name);

        // Normalizes every entry, drops empty ones, sorts and removes duplicates.
        std::vector<std::wstring> NormalizeNameList(const std::vector<std::wstring>& names);

    } // namespace engine
} // namespace mp
