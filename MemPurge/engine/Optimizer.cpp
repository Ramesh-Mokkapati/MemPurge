// MemPurge - engine/Optimizer.cpp
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#include "engine/Optimizer.h"

#include <chrono>
#include <set>
#include <string>

#include "engine/LowPriorityScope.h"
#include "engine/MemoryCombineInformationEx.h"
#include "engine/MemoryStatus.h"
#include "engine/NtApi.h"
#include "engine/Privilege.h"
#include "engine/ProcessName.h"
#include "engine/UniqueHandle.h"

namespace mp
{
    namespace engine
    {
        namespace
        {
            constexpr const wchar_t* kSeProfileSingleProcess = L"SeProfileSingleProcessPrivilege";
            constexpr const wchar_t* kSeDebug = L"SeDebugPrivilege";
            constexpr const wchar_t* kSeIncreaseQuota = L"SeIncreaseQuotaPrivilege";

            // ---- Result helpers --------------------------------------------------------
            void SetWin32Failure(AreaResult& r, DWORD error, AreaStatus status)
            {
                r.status = status;
                r.win32Error = error;
                r.message = SystemErrorMessage(error);
            }

            void SetFromWin32(AreaResult& r, DWORD error)
            {
                if (error == ERROR_SUCCESS)
                {
                    r.status = AreaStatus::Success;
                    r.win32Error = 0;
                    r.message.clear();
                    return;
                }

                SetWin32Failure(r, error, ClassifyWin32Error(error));
            }

            void SetFromNtStatus(AreaResult& r, LONG status)
            {
                r.status = ClassifyNtStatus(status);
                if (r.status == AreaStatus::Success)
                {
                    r.win32Error = 0;
                    r.message.clear();
                    return;
                }

                const NtApi& api = NtApi::Instance();
                r.win32Error = api.HasNtStatusToDosError() ? api.NtStatusToDosError(status)
                    : ERROR_MR_MID_NOT_FOUND;
                r.message = SystemErrorMessage(r.win32Error);
            }

            // Enables each privilege; on failure fills r as PrivilegeMissing and returns false.
            bool RequirePrivileges(AreaResult& r, std::initializer_list<const wchar_t*> privileges)
            {
                for (const wchar_t* name : privileges)
                {
                    const DWORD err = EnablePrivilege(name);
                    if (err != ERROR_SUCCESS)
                    {
                        SetWin32Failure(r, err, AreaStatus::PrivilegeMissing);
                        return false;
                    }
                }

                return true;
            }

            void CallSetSystemInformation(AreaResult& r, ULONG infoClass, PVOID info, ULONG length)
            {
                const NtApi& api = NtApi::Instance();
                if (!api.HasSetSystemInformation())
                {
                    SetWin32Failure(r, ERROR_PROC_NOT_FOUND, AreaStatus::NotSupported);
                    return;
                }

                SetFromNtStatus(r, api.SetSystemInformation(infoClass, info, length));
            }

            void MemoryListCommand(AreaResult& r, ULONG command)
            {
                if (!RequirePrivileges(r, { kSeProfileSingleProcess }))
                {
                    return;
                }

                ULONG cmd = command;
                CallSetSystemInformation(r, kSystemMemoryListInformation, &cmd, sizeof(cmd));
            }

            // ---- Areas -----------------------------------------------------------------
            void TrimWorkingSetsExcluding(AreaResult& r, const std::set<std::wstring>& excluded)
            {
                if (!RequirePrivileges(r, { kSeProfileSingleProcess, kSeDebug }))
                {
                    return;
                }

                const UniqueHandle snapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
                if (!snapshot)
                {
                    SetFromWin32(r, ::GetLastError());
                    return;
                }

                PROCESSENTRY32W entry{};
                entry.dwSize = sizeof(entry);
                if (!::Process32FirstW(snapshot.get(), &entry))
                {
                    SetFromWin32(r, ::GetLastError());
                    return;
                }

                do
                {
                    if (entry.th32ProcessID == 0)
                    {
                        continue;
                    }

                    if (excluded.count(NormalizeProcessName(entry.szExeFile)) != 0)
                    {
                        continue;
                    }

                    const UniqueHandle process(::OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_LIMITED_INFORMATION,
                        FALSE, entry.th32ProcessID));
                    if (!process)
                    {
                        continue; // access denied or process already exited: skip silently
                    }

                    // Per-process failures (access denied, process exiting) are skipped silently.
                    (void)::EmptyWorkingSet(process.get());
                } while (::Process32NextW(snapshot.get(), &entry));

                SetFromWin32(r, ERROR_SUCCESS);
            }

            void WorkingSet(AreaResult& r, const std::set<std::wstring>& excluded)
            {
                if (excluded.empty())
                {
                    MemoryListCommand(r, kMemoryEmptyWorkingSets);
                }
                else
                {
                    TrimWorkingSetsExcluding(r, excluded);
                }
            }

            void SystemFileCache(AreaResult& r)
            {
                if (!RequirePrivileges(r, { kSeIncreaseQuota }))
                {
                    return;
                }

                if (::SetSystemFileCacheSize(static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1), 0))
                {
                    SetFromWin32(r, ERROR_SUCCESS);
                }
                else
                {
                    SetFromWin32(r, ::GetLastError());
                }
            }

            void CombinedPageList(AreaResult& r)
            {
                if (!RequirePrivileges(r, { kSeProfileSingleProcess }))
                {
                    return;
                }

                MemoryCombineInformationEx info{};
                CallSetSystemInformation(r, kSystemCombinePhysicalMemoryInformation, &info,
                    static_cast<ULONG>(sizeof(info)));
            }

            void RegistryCache(AreaResult& r)
            {
                CallSetSystemInformation(r, kSystemRegistryReconciliationInformation, nullptr, 0);
            }

            void ModifiedFileCache(AreaResult& r)
            {
                const DWORD needed = ::GetLogicalDriveStringsW(0, nullptr);
                if (needed == 0)
                {
                    SetFromWin32(r, ::GetLastError());
                    return;
                }

                std::wstring buffer(static_cast<std::size_t>(needed) + 1, L'\0');
                const DWORD len = ::GetLogicalDriveStringsW(static_cast<DWORD>(buffer.size()), &buffer[0]);
                if (len == 0 || len > buffer.size())
                {
                    SetFromWin32(r, len == 0 ? ::GetLastError() : ERROR_INSUFFICIENT_BUFFER);
                    return;
                }

                DWORD firstError = ERROR_SUCCESS;
                std::wstring firstDrive;
                for (const wchar_t* root = buffer.c_str(); *root != L'\0'; root += wcslen(root) + 1)
                {
                    if (root[1] != L':' || ::GetDriveTypeW(root) != DRIVE_FIXED)
                    {
                        continue;
                    }

                    std::wstring volumePath = L"\\\\.\\";
                    volumePath += root[0];
                    volumePath += L':';
                    const UniqueHandle volume(::CreateFileW(volumePath.c_str(), GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                        OPEN_EXISTING, 0, nullptr));
                    DWORD err = ERROR_SUCCESS;
                    if (!volume)
                    {
                        err = ::GetLastError();
                    }
                    else if (!::FlushFileBuffers(volume.get()))
                    {
                        err = ::GetLastError();
                    }

                    if (err != ERROR_SUCCESS && firstError == ERROR_SUCCESS)
                    {
                        firstError = err;
                        firstDrive.assign(root, 2);
                    }
                }

                SetFromWin32(r, firstError);
                if (firstError != ERROR_SUCCESS)
                {
                    r.message = firstDrive + L" " + r.message;
                }
            }

            void RunArea(Area area, AreaResult& r, const std::set<std::wstring>& excluded)
            {
                switch (area)
                {

                case Area::WorkingSet:
                    WorkingSet(r, excluded);
                    break;

                case Area::SystemFileCache:
                    SystemFileCache(r);
                    break;

                case Area::ModifiedPageList:
                    MemoryListCommand(r, kMemoryFlushModifiedList);
                    break;

                case Area::StandbyList:
                    MemoryListCommand(r, kMemoryPurgeStandbyList);
                    break;

                case Area::StandbyListLowPriority:
                    MemoryListCommand(r, kMemoryPurgeLowPriorityStandbyList);
                    break;

                case Area::CombinedPageList:
                    CombinedPageList(r);
                    break;

                case Area::RegistryCache:
                    RegistryCache(r);
                    break;

                case Area::ModifiedFileCache:
                    ModifiedFileCache(r);
                    break;
                }
            }

            std::uint64_t ElapsedMs(std::chrono::steady_clock::time_point start)
            {
                return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start)
                    .count());
            }

            bool AvailablePhysical(std::uint64_t& value)
            {
                MemoryStatus s;
                if (!QueryMemoryStatus(s))
                {
                    return false;
                }

                value = s.availablePhysical;
                return true;
            }

        } // namespace

        RunResult RunOptimization(const RunOptions& options)
        {
            RunResult result;
            const auto runStart = std::chrono::steady_clock::now();
            const LowPriorityScope priority(options.lowPriority);

            const std::vector<Area> areas = AreasFromMask(NormalizeAreaMask(options.areas));
            std::set<std::wstring> excluded;
            for (auto& name : NormalizeNameList(options.exclusions))
            {
                excluded.insert(std::move(name));
            }

            std::uint64_t before = 0;
            const bool haveBefore = AvailablePhysical(before);
            result.results.reserve(areas.size());
            for (std::size_t i = 0; i < areas.size(); ++i)
            {
                if (options.progress)
                {
                    options.progress(i, areas.size(), areas[i]);
                }

                AreaResult r;
                r.area = areas[i];
                const auto areaStart = std::chrono::steady_clock::now();
                RunArea(areas[i], r, excluded);
                r.durationMs = ElapsedMs(areaStart);
                result.results.push_back(std::move(r));
            }

            std::uint64_t after = 0;
            if (haveBefore && AvailablePhysical(after))
            {
                result.freedBytes = ComputeFreedBytes(before, after);
            }

            result.durationMs = ElapsedMs(runStart);
            return result;
        }

    } // namespace engine
} // namespace mp
