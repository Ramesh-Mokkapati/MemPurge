// MemPurge - engine/Privilege.h
#pragma once

namespace mp
{
    namespace engine
    {
        // Enables a privilege (e.g. L"SeDebugPrivilege") in the current process token.
        // Returns ERROR_SUCCESS, or the Win32 error. A FALSE return from
        // AdjustTokenPrivileges or GetLastError() == ERROR_NOT_ALL_ASSIGNED is a failure.
        unsigned long EnablePrivilege(const wchar_t* privilegeName) noexcept;

    } // namespace engine
} // namespace mp
