// MemPurge - core/CommandLine.h
// Command-line parsing (SPEC section 4). Pure; no Windows UI calls.
#pragma once

#include <string>
#include <vector>

#include "core/CommandLineOptions.h"

namespace mp
{
    namespace core
    {
        // args excludes the program name. Switches are case-insensitive and accept
        // '/' or '-' as prefix. Both standby switches -> full StandbyList wins.
        CommandLineOptions ParseCommandLine(const std::vector<std::wstring>& args);

    } // namespace core
} // namespace mp
