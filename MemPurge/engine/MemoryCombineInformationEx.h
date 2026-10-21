// MemPurge - engine/MemoryCombineInformationEx.h
// Buffer for NtSetSystemInformation(SystemCombinePhysicalMemoryInformation)
// (public NT layout, SPEC section 3.1). Plain data.
#pragma once

#include <windows.h>

namespace mp
{
    namespace engine
    {
        struct MemoryCombineInformationEx
        {
            HANDLE Handle;
            ULONG_PTR PagesCombined;
            ULONG Flags;
        };

    } // namespace engine
} // namespace mp
