// MemPurge - core/StartupTask.h
// "Run on startup" through the Task Scheduler 2.0 COM API (SPEC section 5.5).
// The calling thread must have initialized COM.
#pragma once

#include <string>

namespace mp
{
    namespace core
    {
        constexpr const wchar_t* kStartupTaskName = L"MemPurge";

        // True when a task named "MemPurge" exists in the root task folder.
        bool IsStartupTaskRegistered();

        // Creates or replaces the task: logon trigger for the current user,
        // highest run level, action "<exePath>" /Minimized, runs on battery.
        // Returns an HRESULT.
        long RegisterStartupTask(const std::wstring& exePath);

        // Deletes the task. A task that does not exist counts as success.
        long DeleteStartupTask();

    } // namespace core
} // namespace mp
