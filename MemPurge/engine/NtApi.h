// MemPurge - engine/NtApi.h
// ntdll entry points resolved at runtime, and the public NT constants used
// with NtSetSystemInformation (SPEC section 3.1).
#pragma once

#include <windows.h>

namespace mp
{
    namespace engine
    {
        // ---- Public NT constants (SPEC section 3.1) -------------------------------
        const ULONG kSystemMemoryListInformation = 80;
        const ULONG kSystemCombinePhysicalMemoryInformation = 130;
        const ULONG kSystemRegistryReconciliationInformation = 155;

        const ULONG kMemoryEmptyWorkingSets = 2;
        const ULONG kMemoryFlushModifiedList = 3;
        const ULONG kMemoryPurgeStandbyList = 4;
        const ULONG kMemoryPurgeLowPriorityStandbyList = 5;

        // Resolves NtSetSystemInformation and RtlNtStatusToDosError from ntdll once.
        class NtApi
        {
        public:
            static const NtApi& Instance();

            bool HasSetSystemInformation() const noexcept;
            bool HasNtStatusToDosError() const noexcept;

            // Call only when the matching Has...() returns true.
            LONG SetSystemInformation(ULONG infoClass, PVOID info, ULONG length) const;
            ULONG NtStatusToDosError(LONG status) const;

        private:
            typedef LONG(NTAPI* NtSetSystemInformationFn)(ULONG, PVOID, ULONG);
            typedef ULONG(NTAPI* RtlNtStatusToDosErrorFn)(LONG);

            NtApi();

            NtSetSystemInformationFn setSystemInformation_;
            RtlNtStatusToDosErrorFn ntStatusToDosError_;
        };

    } // namespace engine
} // namespace mp
