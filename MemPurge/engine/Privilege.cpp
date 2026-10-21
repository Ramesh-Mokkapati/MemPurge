// MemPurge - engine/Privilege.cpp
#include <windows.h>

#include "engine/Privilege.h"
#include "engine/UniqueHandle.h"

namespace mp
{
    namespace engine
    {
        unsigned long EnablePrivilege(const wchar_t* privilegeName) noexcept
        {
            HANDLE rawToken = nullptr;
            if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &rawToken))
            {
                return ::GetLastError();
            }

            const UniqueHandle token(rawToken);

            LUID luid{};
            if (!::LookupPrivilegeValueW(nullptr, privilegeName, &luid))
            {
                return ::GetLastError();
            }

            TOKEN_PRIVILEGES tp{};
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            ::SetLastError(ERROR_SUCCESS);
            if (!::AdjustTokenPrivileges(token.get(), FALSE, &tp, sizeof(tp), nullptr, nullptr))
            {
                const DWORD err = ::GetLastError();
                return err != ERROR_SUCCESS ? err : ERROR_ACCESS_DENIED;
            }

            const DWORD err = ::GetLastError();
            if (err == ERROR_NOT_ALL_ASSIGNED)
            {
                return ERROR_NOT_ALL_ASSIGNED;
            }

            return ERROR_SUCCESS;
        }

    } // namespace engine
} // namespace mp
