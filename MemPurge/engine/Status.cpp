// MemPurge - engine/Status.cpp
#include <windows.h>

#include "engine/Status.h"

namespace mp
{
    namespace engine
    {
        AreaStatus ClassifyNtStatus(long status) noexcept
        {
            if (status >= 0)
            {
                return AreaStatus::Success;
            }

            switch (status)
            {
            case kStatusInvalidInfoClass:
            case kStatusNotImplemented:
            case kStatusNotSupported:
                return AreaStatus::NotSupported;

            case kStatusPrivilegeNotHeld:
                return AreaStatus::PrivilegeMissing;

            default:
                return AreaStatus::Failed;
            }
        }

        AreaStatus ClassifyWin32Error(unsigned long error) noexcept
        {
            switch (error)
            {
            case ERROR_SUCCESS:
                return AreaStatus::Success;

            case ERROR_NOT_SUPPORTED:
            case ERROR_CALL_NOT_IMPLEMENTED:
            case ERROR_INVALID_LEVEL:
                return AreaStatus::NotSupported;

            case ERROR_PRIVILEGE_NOT_HELD:
            case ERROR_NOT_ALL_ASSIGNED:
                return AreaStatus::PrivilegeMissing;

            default:
                return AreaStatus::Failed;
            }
        }

        std::wstring SystemErrorMessage(unsigned long error)
        {
            wchar_t* buffer = nullptr;
            const DWORD len = ::FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, error, 0, reinterpret_cast<wchar_t*>(&buffer), 0, nullptr);
            std::wstring text;
            if (len != 0 && buffer != nullptr)
            {
                text.assign(buffer, len);
            }

            if (buffer != nullptr)
            {
                ::LocalFree(buffer);
            }

            while (!text.empty() &&
                (text.back() == L'\r' || text.back() == L'\n' || text.back() == L' ' || text.back() == L'.'))
            {
                text.pop_back();
            }

            return text;
        }

    } // namespace engine
} // namespace mp
