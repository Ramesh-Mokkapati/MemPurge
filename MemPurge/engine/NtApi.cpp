// MemPurge - engine/NtApi.cpp
#include "engine/NtApi.h"

namespace mp
{
    namespace engine
    {
        NtApi::NtApi()
            : setSystemInformation_(nullptr)
            , ntStatusToDosError_(nullptr)
        {
            const HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");
            if (ntdll != nullptr)
            {
                setSystemInformation_ = reinterpret_cast<NtSetSystemInformationFn>(
                    reinterpret_cast<void*>(::GetProcAddress(ntdll, "NtSetSystemInformation")));
                ntStatusToDosError_ = reinterpret_cast<RtlNtStatusToDosErrorFn>(
                    reinterpret_cast<void*>(::GetProcAddress(ntdll, "RtlNtStatusToDosError")));
            }
        }

        const NtApi& NtApi::Instance()
        {
            // Thread-safe in C++11 ("magic statics").
            static const NtApi api;
            return api;
        }

        bool NtApi::HasSetSystemInformation() const noexcept
        {
            return setSystemInformation_ != nullptr;
        }

        bool NtApi::HasNtStatusToDosError() const noexcept
        {
            return ntStatusToDosError_ != nullptr;
        }

        LONG NtApi::SetSystemInformation(ULONG infoClass, PVOID info, ULONG length) const
        {
            return setSystemInformation_(infoClass, info, length);
        }

        ULONG NtApi::NtStatusToDosError(LONG status) const
        {
            return ntStatusToDosError_(status);
        }

    } // namespace engine
} // namespace mp
