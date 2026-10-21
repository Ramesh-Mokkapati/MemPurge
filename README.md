# MemPurge

MemPurge is a small native Windows tray utility (C++11 / MFC, statically linked) that frees RAM
on demand or automatically. It asks Windows to trim process working sets and to flush or purge
memory caches, using only public Windows APIs.

MemPurge requires administrator rights. Its manifest requests elevation (`requireAdministrator`).

## Features

- Eight memory areas, run in this order. Each one is optional, and a failing area does not stop
  the others:
  1. **Working set**: trims the working sets of processes (excluded processes are skipped)
  2. **System file cache**: flushes the system file cache
  3. **Modified page list**: writes modified pages to disk
  4. **Standby list**: purges the whole standby list
  5. **Standby list (low priority)**: purges only priority-0 standby pages (cannot be combined
     with the full standby list)
  6. **Combined page list**: flushes page-combining memory
  7. **Registry cache**: reconciles and flushes registry hives
  8. **Modified file cache**: commits pending writes on every fixed volume
- Live physical memory display, plus optional commit (virtual) memory, refreshed every second.
- Notification-area icon that can show the used-memory percentage, coloured green, amber
  (warning level), red (danger level) or grey (optimizing). It also offers a balloon after each
  run, left-click show/hide, an optional middle-click optimize, and a right-click menu.
- Automatic optimization every N hours, and/or when free memory falls below N %. Free memory is
  checked every 5 s, and low-memory runs are at least 5 minutes apart.
- Global hotkey (default Ctrl+Shift+M, off by default).
- Process exclusion list for working-set trimming.
- Run on startup through a Task Scheduler logon task that runs with highest privileges.
- Start minimized, close to notification area, always on top, low-priority runs.
- Settings are stored in `HKEY_CURRENT_USER\Software\MemPurge`.

## Command line

Any area switch makes MemPurge run headless. It creates no window and no tray icon, and it skips
the single-instance check.

```
MemPurge.exe [area switches] [/Minimized] [/?]

  /WorkingSet              Trim the working sets of processes
  /SystemFileCache         Flush the system file cache
  /ModifiedPageList        Write modified pages to disk
  /StandbyList             Purge the standby list
  /StandbyListLowPriority  Purge low-priority standby pages only
  /CombinedPageList        Flush the combined page list
  /RegistryCache           Flush registry hives
  /ModifiedFileCache       Commit pending writes on fixed volumes
  /Minimized               (GUI) start in the notification area only
  /? or /Help              Show usage
```

- Switches are case-insensitive and may start with `/` or `-`.
- If both standby switches are given, the full `/StandbyList` wins.
- The process exclusion list from the settings also applies in command-line mode.
- Output: when MemPurge is started from a console, it prints one line per area and a final
  `Freed: X` line. MemPurge is a GUI-subsystem program, so `cmd.exe` does not wait for it. Use
  `start /wait MemPurge.exe /StandbyList` to see the output in order and to read the exit code.
- Exit codes: `0` means every requested area succeeded. `1` means an area failed or is not
  supported. `2` means an unknown switch was given; MemPurge then prints the usage.

Example (from an elevated prompt):

```
start /wait MemPurge.exe /WorkingSet /StandbyList /ModifiedPageList
echo %ERRORLEVEL%
```

## Windows APIs used

MemPurge uses only Windows APIs. The memory engine (`MemPurge/engine/`) uses plain Win32 and
does not depend on MFC. It is linked with `psapi.lib`, `taskschd.lib`, `secur32.lib`,
`uxtheme.lib` and `comctl32.lib`. `ntdll.dll` exports are resolved at runtime with
`GetModuleHandleW` + `GetProcAddress`, so there is no import-library dependency on ntdll.

### Memory areas

| Area | API call | Privilege enabled first |
|---|---|---|
| Working set (no exclusions) | `NtSetSystemInformation(SystemMemoryListInformation = 80, MemoryEmptyWorkingSets = 2)` | `SeProfileSingleProcessPrivilege` |
| Working set (with exclusions) | `CreateToolhelp32Snapshot` / `Process32FirstW` / `Process32NextW` to list processes; for each non-excluded one, `OpenProcess(PROCESS_SET_QUOTA \| PROCESS_QUERY_LIMITED_INFORMATION)` and then `EmptyWorkingSet` (psapi) | `SeProfileSingleProcessPrivilege`, `SeDebugPrivilege` |
| System file cache | `SetSystemFileCacheSize((SIZE_T)-1, (SIZE_T)-1, 0)`, which flushes the cache | `SeIncreaseQuotaPrivilege` |
| Modified page list | `NtSetSystemInformation(SystemMemoryListInformation, MemoryFlushModifiedList = 3)` | `SeProfileSingleProcessPrivilege` |
| Standby list | `NtSetSystemInformation(SystemMemoryListInformation, MemoryPurgeStandbyList = 4)` | `SeProfileSingleProcessPrivilege` |
| Standby list (low priority) | `NtSetSystemInformation(SystemMemoryListInformation, MemoryPurgeLowPriorityStandbyList = 5)` | `SeProfileSingleProcessPrivilege` |
| Combined page list | `NtSetSystemInformation(SystemCombinePhysicalMemoryInformation = 130, MEMORY_COMBINE_INFORMATION_EX)` with a zeroed `{ Handle, PagesCombined, Flags }` struct | `SeProfileSingleProcessPrivilege` |
| Registry cache | `NtSetSystemInformation(SystemRegistryReconciliationInformation = 155, NULL, 0)` | none |
| Modified file cache | `GetLogicalDriveStringsW` + `GetDriveTypeW` (`DRIVE_FIXED` only), then for each volume `CreateFileW(L"\\\\.\\X:", GENERIC_READ \| GENERIC_WRITE)` and `FlushFileBuffers` | none (administrator) |

The `NtSetSystemInformation` information classes and command values are public NT constants
(see the phnt headers). Microsoft documents `SetSystemFileCacheSize`, `EmptyWorkingSet` and
`FlushFileBuffers` on a volume handle.

### Privileges, errors and status

- **Privileges:** `OpenProcessToken(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY)`,
  `LookupPrivilegeValueW`, `AdjustTokenPrivileges`. An area whose privilege can't be enabled is
  reported as *Privilege missing*. That includes the case where the call succeeds but
  `GetLastError()` returns `ERROR_NOT_ALL_ASSIGNED`.
- **Error decoding:** `NtSetSystemInformation` returns an `NTSTATUS`. It is converted with
  `RtlNtStatusToDosError` (ntdll) and then turned into text with `FormatMessageW`.
  `STATUS_INVALID_INFO_CLASS`, `STATUS_NOT_IMPLEMENTED` and `STATUS_NOT_SUPPORTED` are reported
  as *Not supported*.
- **Memory status:** `GlobalMemoryStatusEx`. Physical usage comes from `ullTotalPhys` /
  `ullAvailPhys`, and commit usage from `ullTotalPageFile` / `ullAvailPageFile`. "Freed" is the
  increase in `ullAvailPhys` across a run.
- **OS version:** `RtlGetVersion` (ntdll), which is not affected by manifest-based version
  shimming.
- **Low-priority runs:** `SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL)` and
  `SetPriorityClass(PROCESS_MODE_BACKGROUND_BEGIN / _END)` for the duration of a run.
- **Process names:** `LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_LOWERCASE)` lower-cases names
  for exclusion matching. `CompareStringOrdinal` compares command-line switches without regard
  to case.

### Application and shell

| Feature | APIs |
|---|---|
| Notification-area icon | `Shell_NotifyIconW` (`NIM_ADD` / `NIM_MODIFY` / `NIM_DELETE` / `NIM_SETVERSION` with `NOTIFYICON_VERSION_4`, `NIF_INFO` balloons). The icon is re-added on the `TaskbarCreated` message (`RegisterWindowMessageW`). |
| Usage % icon | GDI: `CreateCompatibleDC`, `CreateCompatibleBitmap`, `CreateBitmap` (mask), `CreateSolidBrush`, `CreateFontW`, `DrawTextW`, then `CreateIconIndirect` / `DestroyIcon`. The size comes from `GetSystemMetricsForDpi(SM_CXSMICON)` with `GetDpiForSystem`. |
| Messages to an elevated window | `ChangeWindowMessageFilterEx` lets Explorer's tray callback and `TaskbarCreated` messages through to the elevated window. |
| Global hotkey | `RegisterHotKey` / `UnregisterHotKey` (`MOD_NOREPEAT`), handled in `WM_HOTKEY`. |
| Timers and power | MFC `SetTimer`/`KillTimer`, `GetTickCount64`; `WM_POWERBROADCAST` (`PBT_APMRESUMEAUTOMATIC`) restarts the timers. |
| Single instance | `CreateMutexW(L"Local\\MemPurge.SingleInstance")`. The window handle is published in a named file mapping (`CreateFileMappingW` / `OpenFileMappingW` / `MapViewOfFile`). A second instance calls `AllowSetForegroundWindow` and posts a registered activation message. |
| Run on startup | Task Scheduler 2.0 COM (`CoCreateInstance(CLSID_TaskScheduler)` → `ITaskService`, `ITaskFolder`, `ITaskDefinition`). It creates a `TASK_TRIGGER_LOGON` trigger for the current user (`GetUserNameExW`), `TASK_RUNLEVEL_HIGHEST`, `TASK_LOGON_INTERACTIVE_TOKEN` and `TASK_INSTANCES_IGNORE_NEW`, with no execution time limit and no battery restrictions. It is saved with `RegisterTaskDefinition(TASK_CREATE_OR_UPDATE)` and removed with `DeleteTask`. |
| Settings | `RegCreateKeyExW` / `RegOpenKeyExW` / `RegQueryValueExW` / `RegSetValueExW` under `HKCU\Software\MemPurge`, using `REG_DWORD` values and a `REG_MULTI_SZ` exclusion list. |
| Command-line mode | `GetCommandLineW` + `CommandLineToArgvW`. `AttachConsole(ATTACH_PARENT_PROCESS)` attaches to the parent console. Output goes through `WriteConsoleW`, or `WriteFile` as UTF-8 (`WideCharToMultiByte`) when it is redirected. |
| UI | MFC dialogs (`CDialog`, `CTabCtrl`, `CHotKeyCtrl`), `InitCommonControlsEx`, `EnableThemeDialogTexture` (uxtheme), `LoadIconMetric` (comctl32). |

## Building

Requirements: Visual Studio 2022 or later with the v143 toolset and the **C++ ATL/MFC**
component, and a Windows 10/11 SDK.

```
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" ^
    MemPurge.sln /m /p:Configuration=Release /p:Platform=x64
```

Valid configurations are `Debug` and `Release`, with platforms `x64` and `x86` (`Win32`). The
output goes to `bin\<Platform>\<Configuration>\`, and intermediate files go to `obj\`.

VS 2026 builds the solution as is, as long as its v143 toolset component is installed. To use
VS 2026's own toolset instead, pass `/p:PlatformToolset=v145`.

If the checkout path is very long, intermediate paths can exceed MAX_PATH. Map a short drive
first:

```
subst P: "C:\path\to\MemPurge"
MSBuild P:\MemPurge.sln /p:Configuration=Release /p:Platform=x64
subst P: /D
```

### Tests

`MemPurgeTests.exe` (built with the solution) is a console program that tests command-line
parsing, size formatting, settings normalization, exclusion-name normalization and the area
bitmask. It links only pure helpers and never performs a memory operation:

```
bin\x64\Release\MemPurgeTests.exe
```

### Continuous integration

[`.github/workflows/build.yml`](.github/workflows/build.yml) runs on pushes to `main`, on pull
requests and on demand. It uses a `windows-2022` runner (Visual Studio 2022 with ATL/MFC) and:

- builds `Debug` and `Release` for both `x64` and `x86` with `-warnaserror`, so any warning fails
  the build
- runs `MemPurgeTests.exe` for each configuration (MemPurge.exe itself is never started)
- uploads `MemPurge.exe` from each Release build as the `MemPurge-x64` and `MemPurge-x86`
  artifacts

## Layout

`MemPurge.sln`, `README.md` and `.gitignore` are at the top level. Everything else is under `MemPurge/`:

| Folder        | Contents                                                                 |
|---------------|--------------------------------------------------------------------------|
| `MemPurge/engine/` | Memory operations, privileges, memory status, OS version (Win32 + STL)    |
| `MemPurge/core/`   | Settings (registry), command-line parsing, size formatting, startup task  |
| `MemPurge/ui/`     | MFC application, dialogs, tray icon, hotkey, timers, resources, manifest  |
| `MemPurge/tests/`  | Console test executable                                                   |
| `MemPurge/docs/`   | Specification, clean-room record, implementation notes                    |

`MemPurge/Common.props` holds the compiler and linker settings shared by both projects.

## License

MIT. See [LICENSE](MemPurge/LICENSE).
