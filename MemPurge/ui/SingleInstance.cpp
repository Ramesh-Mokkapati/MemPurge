// MemPurge - ui/SingleInstance.cpp
#include "framework.h"

#include "SingleInstance.h"

#include "MappedView.h"

#include <cstdint>

namespace mp
{
    namespace ui
    {
        namespace
        {
            constexpr const wchar_t* kMutexName = L"Local\\MemPurge.SingleInstance";
            constexpr const wchar_t* kMappingName = L"Local\\MemPurge.MainWindow";
            constexpr const wchar_t* kActivateMessageName = L"MemPurge.Activate";
        } // namespace

        bool SingleInstance::Acquire()
        {
            mutex_.reset(::CreateMutexW(nullptr, FALSE, kMutexName));
            if (!mutex_)
            {
                return true; // cannot tell; behave as the first instance
            }

            return ::GetLastError() != ERROR_ALREADY_EXISTS;
        }

        void SingleInstance::PublishWindow(HWND hwnd)
        {
            mapping_.reset(::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                sizeof(std::uint64_t), kMappingName));
            if (!mapping_)
            {
                return;
            }

            const MappedView view(mapping_.get(), FILE_MAP_WRITE);
            if (view.get() != nullptr)
            {
                *view.get() = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(hwnd));
            }
        }

        void SingleInstance::ActivateExisting()
        {
            const engine::UniqueHandle mapping(::OpenFileMappingW(FILE_MAP_READ, FALSE, kMappingName));
            if (!mapping)
            {
                return;
            }

            HWND hwnd = nullptr;
            {
                const MappedView view(mapping.get(), FILE_MAP_READ);
                if (view.get() == nullptr)
                {
                    return;
                }

                hwnd = reinterpret_cast<HWND>(static_cast<std::uintptr_t>(*view.get()));
            }

            if (hwnd == nullptr || !::IsWindow(hwnd))
            {
                return;
            }

            DWORD pid = 0;
            ::GetWindowThreadProcessId(hwnd, &pid);
            if (pid != 0)
            {
                ::AllowSetForegroundWindow(pid);
            }

            ::PostMessageW(hwnd, ActivateMessage(), 0, 0);
        }

        UINT SingleInstance::ActivateMessage()
        {
            static const UINT message = ::RegisterWindowMessageW(kActivateMessageName);
            return message;
        }

    } // namespace ui
} // namespace mp
