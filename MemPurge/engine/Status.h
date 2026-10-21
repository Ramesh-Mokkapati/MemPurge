// MemPurge - engine/Status.h
// Result status of one memory area and classification of NTSTATUS / Win32 codes.
#pragma once

#include <cstdint>
#include <string>

namespace mp
{
    namespace engine
    {
        enum class AreaStatus
        {
            Success,
            NotSupported,
            PrivilegeMissing,
            Failed,
        };

        // NTSTATUS values used by the classification (public values from ntstatus.h).
        constexpr long kStatusSuccess = 0x00000000L;
        constexpr long kStatusNotImplemented = static_cast<long>(0xC0000002UL);
        constexpr long kStatusInvalidInfoClass = static_cast<long>(0xC0000003UL);
        constexpr long kStatusAccessDenied = static_cast<long>(0xC0000022UL);
        constexpr long kStatusPrivilegeNotHeld = static_cast<long>(0xC0000061UL);
        constexpr long kStatusNotSupported = static_cast<long>(0xC00000BBUL);

        // Pure: maps an NTSTATUS to an AreaStatus.
        //   NT_SUCCESS                                   -> Success
        //   INVALID_INFO_CLASS / NOT_IMPLEMENTED / NOT_SUPPORTED -> NotSupported
        //   PRIVILEGE_NOT_HELD                           -> PrivilegeMissing
        //   anything else                                -> Failed
        AreaStatus ClassifyNtStatus(long status) noexcept;

        // Pure: maps a Win32 error code to an AreaStatus (0 -> Success).
        AreaStatus ClassifyWin32Error(unsigned long error) noexcept;

        // System message text for a Win32 error / HRESULT (FormatMessageW), trailing
        // whitespace and periods trimmed. Empty when the system has no text.
        std::wstring SystemErrorMessage(unsigned long error);

    } // namespace engine
} // namespace mp
