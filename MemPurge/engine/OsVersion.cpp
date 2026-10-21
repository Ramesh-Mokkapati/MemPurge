// MemPurge - engine/OsVersion.cpp
#include <windows.h>

#include "engine/OsVersion.h"

namespace mp
{
    namespace engine
    {
        namespace
        {
            using RtlGetVersionFn = LONG(NTAPI*)(PRTL_OSVERSIONINFOW);
        }

        OsVersion GetOsVersion() noexcept
        {
            OsVersion v;
            const HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");
            if (ntdll == nullptr)
            {
                return v;
            }

            const FARPROC proc = ::GetProcAddress(ntdll, "RtlGetVersion");
            if (proc == nullptr)
            {
                return v;
            }

            const auto rtlGetVersion = reinterpret_cast<RtlGetVersionFn>(reinterpret_cast<void*>(proc));
            RTL_OSVERSIONINFOW info{};
            info.dwOSVersionInfoSize = sizeof(info);
            if (rtlGetVersion(&info) >= 0)
            {
                v.major = info.dwMajorVersion;
                v.minor = info.dwMinorVersion;
                v.build = info.dwBuildNumber;
            }

            return v;
        }

        bool IsWindows10OrLater() noexcept
        {
            return GetOsVersion().major >= 10;
        }

    } // namespace engine
} // namespace mp
