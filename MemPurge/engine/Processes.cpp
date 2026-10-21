// MemPurge - engine/Processes.cpp
#include <windows.h>
#include <tlhelp32.h>

#include "engine/Processes.h"
#include "engine/ProcessName.h"
#include "engine/UniqueHandle.h"

namespace mp
{
    namespace engine
    {
        std::vector<std::wstring> EnumerateProcessNames()
        {
            std::vector<std::wstring> names;
            const UniqueHandle snapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
            if (!snapshot)
            {
                return names;
            }

            PROCESSENTRY32W entry{};
            entry.dwSize = sizeof(entry);
            for (BOOL ok = ::Process32FirstW(snapshot.get(), &entry); ok;
                ok = ::Process32NextW(snapshot.get(), &entry))
            {
                if (entry.th32ProcessID == 0)
                {
                    continue; // System Idle Process
                }

                names.emplace_back(entry.szExeFile);
            }

            return NormalizeNameList(names);
        }

    } // namespace engine
} // namespace mp
