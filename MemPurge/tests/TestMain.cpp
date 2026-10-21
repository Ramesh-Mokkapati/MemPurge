// MemPurge - tests/TestMain.cpp
// Console tests for core/ and the pure engine/ helpers (SPEC section 7).
// No framework. Never calls a memory operation: the engine Optimizer and the
// registry / Task Scheduler code are not linked into this executable.
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "core/CommandLine.h"
#include "core/Settings.h"
#include "core/SizeFormat.h"
#include "engine/Areas.h"
#include "engine/MemoryStatus.h"
#include "engine/ProcessName.h"
#include "engine/Status.h"

namespace
{
    int g_checks = 0;
    int g_failures = 0;

#define CHECK(expr)                                                                \
    do {                                                                           \
        ++g_checks;                                                                \
        if (!(expr)) {                                                             \
            ++g_failures;                                                          \
            std::printf("  FAIL %s(%d): %s\n", __FILE__, __LINE__, #expr);          \
        }                                                                          \
    } while (0)

#define CHECK_EQ_WSTR(actual, expected)                                            \
    do {                                                                           \
        ++g_checks;                                                                \
        const std::wstring a_ = (actual);                                          \
        const std::wstring e_ = (expected);                                        \
        if (a_ != e_) {                                                            \
            ++g_failures;                                                          \
            std::printf("  FAIL %s(%d): %s == \"%ls\", expected \"%ls\"\n",        \
                        __FILE__, __LINE__, #actual, a_.c_str(), e_.c_str());      \
        }                                                                          \
    } while (0)

    using mp::engine::Area;
    using mp::engine::ToMask;

    void Section(const char* name)
    {
        std::printf("[%s]\n", name);
    }

    // ---------------------------------------------------------------------------
    void TestCommandLine()
    {
        Section("CommandLine");
        using mp::core::ParseCommandLine;

        {
            const auto o = ParseCommandLine({});
            CHECK(o.areas == 0);
            CHECK(!o.help && !o.minimized && !o.HasError() && !o.IsHeadless());
        }

        // Case-insensitivity and both prefixes.
        {
            const auto o = ParseCommandLine({ L"/workingset", L"-SYSTEMFILECACHE", L"/ModifiedPageList" });
            CHECK(!o.HasError());
            CHECK(o.IsHeadless());
            CHECK(o.areas == (ToMask(Area::WorkingSet) | ToMask(Area::SystemFileCache) |
                ToMask(Area::ModifiedPageList)));
        }

        {
            const auto o = ParseCommandLine({ L"-cOmBiNeDpAgElIsT", L"/registrycache", L"-ModifiedFileCache" });
            CHECK(o.areas == (ToMask(Area::CombinedPageList) | ToMask(Area::RegistryCache) |
                ToMask(Area::ModifiedFileCache)));
        }

        // Every documented switch name is accepted.
        for (Area a : mp::engine::kAreaOrder)
        {
            const std::wstring sw = L"/" + std::wstring(mp::engine::AreaSwitchName(a));
            const auto o = ParseCommandLine({ sw });
            CHECK(!o.HasError());
            CHECK(o.areas == ToMask(a));
        }

        // Standby precedence: full StandbyList wins, in either order.
        {
            const auto o = ParseCommandLine({ L"/StandbyList", L"/StandbyListLowPriority" });
            CHECK(o.areas == ToMask(Area::StandbyList));
        }

        {
            const auto o = ParseCommandLine({ L"-standbylistlowpriority", L"-STANDBYLIST" });
            CHECK(o.areas == ToMask(Area::StandbyList));
        }

        {
            const auto o = ParseCommandLine({ L"/StandbyListLowPriority" });
            CHECK(o.areas == ToMask(Area::StandbyListLowPriority));
        }

        // Unknown switches.
        {
            const auto o = ParseCommandLine({ L"/WorkingSet", L"/Bogus" });
            CHECK(o.HasError());
            CHECK(o.unknownSwitches.size() == 1 && o.unknownSwitches[0] == L"/Bogus");
        }

        {
            const auto o = ParseCommandLine({ L"WorkingSet" }); // no prefix
            CHECK(o.HasError());
        }

        {
            const auto o = ParseCommandLine({ L"/" });
            CHECK(o.HasError());
        }

        {
            const auto o = ParseCommandLine({ L"--WorkingSet" });
            CHECK(o.HasError());
        }

        // Help.
        CHECK(ParseCommandLine({ L"/?" }).help);
        CHECK(ParseCommandLine({ L"-?" }).help);
        CHECK(ParseCommandLine({ L"/Help" }).help);
        CHECK(ParseCommandLine({ L"-HELP" }).help);
        CHECK(!ParseCommandLine({ L"/?" }).HasError());

        // Minimized is not an area switch.
        {
            const auto o = ParseCommandLine({ L"/minimized" });
            CHECK(o.minimized && !o.IsHeadless() && !o.HasError());
        }

        // Empty arguments are ignored.
        CHECK(!ParseCommandLine({ L"" }).HasError());
    }

    // ---------------------------------------------------------------------------
    void TestFormatBytes()
    {
        Section("FormatBytes");
        using mp::core::FormatBytes;
        constexpr std::uint64_t KB = 1024ull;
        constexpr std::uint64_t MB = KB * 1024;
        constexpr std::uint64_t GB = MB * 1024;
        constexpr std::uint64_t TB = GB * 1024;

        CHECK_EQ_WSTR(FormatBytes(0), L"0 B");
        CHECK_EQ_WSTR(FormatBytes(1), L"1 B");
        CHECK_EQ_WSTR(FormatBytes(1023), L"1023 B");
        CHECK_EQ_WSTR(FormatBytes(1024), L"1.0 KB");
        CHECK_EQ_WSTR(FormatBytes(1536), L"1.5 KB");
        CHECK_EQ_WSTR(FormatBytes(1075), L"1.0 KB");          // 1.0498 -> 1.0
        CHECK_EQ_WSTR(FormatBytes(1076), L"1.1 KB");          // 1.0508 -> 1.1
        CHECK_EQ_WSTR(FormatBytes(MB - 1), L"1.0 MB");        // 1023.999 KB rounds up -> promoted
        CHECK_EQ_WSTR(FormatBytes(MB - 52), L"1023.9 KB");
        CHECK_EQ_WSTR(FormatBytes(MB), L"1.0 MB");
        CHECK_EQ_WSTR(FormatBytes(MB * 3 / 2), L"1.5 MB");
        CHECK_EQ_WSTR(FormatBytes(GB), L"1.0 GB");
        CHECK_EQ_WSTR(FormatBytes(GB * 16), L"16.0 GB");
        CHECK_EQ_WSTR(FormatBytes(GB * 5 / 4), L"1.3 GB");    // 1.25 rounds half up
        CHECK_EQ_WSTR(FormatBytes(TB - 1), L"1.0 TB");
        CHECK_EQ_WSTR(FormatBytes(TB), L"1.0 TB");
        CHECK_EQ_WSTR(FormatBytes(TB * 2048), L"2048.0 TB");  // TB is the largest unit
        CHECK_EQ_WSTR(FormatBytes(UINT64_MAX), L"16777216.0 TB");

        const mp::core::ByteUnitNames custom{ L"o", L"Kio", L"Mio", L"Gio", L"Tio" };
        CHECK_EQ_WSTR(FormatBytes(512, custom), L"512 o");
        CHECK_EQ_WSTR(FormatBytes(1536, custom), L"1.5 Kio");
    }

    // ---------------------------------------------------------------------------
    void TestSettings()
    {
        Section("Settings");
        using namespace mp::core;

        const Settings d = DefaultSettings();
        CHECK(d.areas == (mp::engine::kAllAreasMask & ~ToMask(Area::StandbyListLowPriority)));
        CHECK(d.intervalHours == 0);
        CHECK(d.lowMemoryPercent == 0);
        CHECK(!d.hotkeyEnabled);
        CHECK(d.hotkey == MakeHotkey('M', kHotkeyfControl | kHotkeyfShift));
        CHECK(d.showNotifications);
        CHECK(d.closeToTray);
        CHECK(d.showUsageInTray);
        CHECK(d.warningLevel == 80);
        CHECK(d.dangerLevel == 90);
        CHECK(!d.startMinimized && !d.alwaysOnTop && !d.showVirtualMemory && !d.lowPriority &&
            !d.optimizeOnMiddleClick);
        CHECK(d.exclusions.empty());

        // Defaults are already normalized.
        {
            Settings n = d;
            NormalizeSettings(n);
            CHECK(n == d);
        }

        // Out-of-range values are clamped.
        {
            Settings s = d;
            s.areas = 0xFFFFFFFFu;
            s.intervalHours = 25;
            s.lowMemoryPercent = 100;
            s.warningLevel = 0;
            s.dangerLevel = 500;
            NormalizeSettings(s);
            CHECK(s.areas == (mp::engine::kAllAreasMask & ~ToMask(Area::StandbyListLowPriority)));
            CHECK(s.intervalHours == 24);
            CHECK(s.lowMemoryPercent == 99);
            CHECK(s.warningLevel == 1);
            CHECK(s.dangerLevel == 100);
        }

        {
            Settings s = d;
            s.intervalHours = 24;
            s.lowMemoryPercent = 99;
            NormalizeSettings(s);
            CHECK(s.intervalHours == 24);
            CHECK(s.lowMemoryPercent == 99);
        }

        // Warning must not exceed danger.
        {
            Settings s = d;
            s.warningLevel = 95;
            s.dangerLevel = 85;
            NormalizeSettings(s);
            CHECK(s.warningLevel == 85);
            CHECK(s.dangerLevel == 85);
        }

        // Low-priority standby alone survives; both -> full standby.
        {
            Settings s = d;
            s.areas = ToMask(Area::StandbyListLowPriority);
            NormalizeSettings(s);
            CHECK(s.areas == ToMask(Area::StandbyListLowPriority));
        }

        // Hotkey: missing virtual key -> default; unknown modifier bits dropped.
        {
            Settings s = d;
            s.hotkey = MakeHotkey(0, kHotkeyfAlt);
            NormalizeSettings(s);
            CHECK(s.hotkey == kDefaultHotkey);
            s.hotkey = MakeHotkey('K', 0xF0 | kHotkeyfAlt);
            NormalizeSettings(s);
            CHECK(s.hotkey == MakeHotkey('K', kHotkeyfAlt));
        }

        // Exclusions are normalized, deduplicated and sorted.
        {
            Settings s = d;
            s.exclusions = { L"Notepad.exe", L" chrome.EXE ", L"notepad", L"", L"   " };
            NormalizeSettings(s);
            CHECK(s.exclusions.size() == 2);
            CHECK(s.exclusions.size() == 2 && s.exclusions[0] == L"chrome" && s.exclusions[1] == L"notepad");
        }

        // Hotkey packing and modifier conversion.
        CHECK(HotkeyVirtualKey(MakeHotkey('M', 3)) == 'M');
        CHECK(HotkeyModifiers(MakeHotkey('M', 3)) == 3);
        CHECK(HotkeyfToModFlags(kHotkeyfControl | kHotkeyfShift) == (MOD_CONTROL | MOD_SHIFT));
        CHECK(HotkeyfToModFlags(kHotkeyfAlt) == MOD_ALT);
        CHECK(HotkeyfToModFlags(kHotkeyfExt) == 0);

        // REG_MULTI_SZ round-trip.
        {
            const std::vector<std::wstring> values{ L"chrome", L"notepad" };
            const std::wstring raw = BuildMultiSz(values);
            CHECK(raw.size() == 6 + 1 + 7 + 1 + 1);
            CHECK(ParseMultiSz(raw.data(), raw.size()) == values);
            const std::wstring emptyRaw = BuildMultiSz({});
            CHECK(emptyRaw.size() == 2);
            CHECK(ParseMultiSz(emptyRaw.data(), emptyRaw.size()).empty());
            const wchar_t unterminated[] = { L'a', L'b' };
            CHECK(ParseMultiSz(unterminated, 2) == std::vector<std::wstring>{L"ab"});
            CHECK(ParseMultiSz(nullptr, 0).empty());
        }
    }

    // ---------------------------------------------------------------------------
    void TestProcessNames()
    {
        Section("ProcessName");
        using mp::engine::NormalizeProcessName;
        CHECK_EQ_WSTR(NormalizeProcessName(L"Chrome.EXE "), L"chrome");
        CHECK_EQ_WSTR(NormalizeProcessName(L"  notepad  "), L"notepad");
        CHECK_EQ_WSTR(NormalizeProcessName(L"notepad.exe"), L"notepad");
        CHECK_EQ_WSTR(NormalizeProcessName(L"C:\\Program Files\\App\\MyApp.Exe"), L"myapp");
        CHECK_EQ_WSTR(NormalizeProcessName(L"a.exe.exe"), L"a.exe");
        CHECK_EQ_WSTR(NormalizeProcessName(L"svchost.dll"), L"svchost.dll");
        CHECK_EQ_WSTR(NormalizeProcessName(L".exe"), L"");
        CHECK_EQ_WSTR(NormalizeProcessName(L""), L"");
        CHECK_EQ_WSTR(NormalizeProcessName(L"\t"), L"");

        const auto list = mp::engine::NormalizeNameList({ L"B.exe", L"a", L"b", L" A.EXE" });
        CHECK(list == (std::vector<std::wstring>{L"a", L"b"}));
    }

    // ---------------------------------------------------------------------------
    void TestAreas()
    {
        Section("Areas");
        using namespace mp::engine;

        // Execution order matches the SPEC table.
        const std::vector<Area> expected{ Area::WorkingSet,       Area::SystemFileCache,
                                         Area::ModifiedPageList, Area::StandbyList,
                                         Area::StandbyListLowPriority, Area::CombinedPageList,
                                         Area::RegistryCache,    Area::ModifiedFileCache };
        CHECK(AreasFromMask(kAllAreasMask) == expected);
        CHECK(AreasFromMask(0).empty());
        for (std::size_t i = 0; i < kAreaOrder.size(); ++i)
        {
            CHECK(AreaIndex(kAreaOrder[i]) == i);
        }

        // Order is independent of bit order: higher bits never come first.
        {
            const auto v = AreasFromMask(ToMask(Area::ModifiedFileCache) | ToMask(Area::WorkingSet) |
                ToMask(Area::RegistryCache));
            CHECK(v.size() == 3 && v[0] == Area::WorkingSet && v[1] == Area::RegistryCache &&
                v[2] == Area::ModifiedFileCache);
        }

        // Round trip for every subset; the list is strictly in execution order.
        bool allRoundTrip = true;
        bool allOrdered = true;
        for (std::uint32_t mask = 0; mask <= kAllAreasMask; ++mask)
        {
            const auto areas = AreasFromMask(mask);
            if (MaskFromAreas(areas) != mask)
            {
                allRoundTrip = false;
            }

            for (std::size_t i = 1; i < areas.size(); ++i)
            {
                if (AreaIndex(areas[i - 1]) >= AreaIndex(areas[i]))
                {
                    allOrdered = false;
                }
            }
        }

        CHECK(allRoundTrip);
        CHECK(allOrdered);

        // Unknown bits are ignored by AreasFromMask and dropped by NormalizeAreaMask.
        CHECK(AreasFromMask(0xFFFFFF00u).empty());
        CHECK(NormalizeAreaMask(0xFFFFFF01u) == ToMask(Area::WorkingSet));
        CHECK(NormalizeAreaMask(ToMask(Area::StandbyList) | ToMask(Area::StandbyListLowPriority)) ==
            ToMask(Area::StandbyList));
        CHECK(NormalizeAreaMask(ToMask(Area::StandbyListLowPriority)) == ToMask(Area::StandbyListLowPriority));
    }

    // ---------------------------------------------------------------------------
    void TestStatusAndMemoryMath()
    {
        Section("Status / memory math");
        using namespace mp::engine;

        CHECK(ClassifyNtStatus(kStatusSuccess) == AreaStatus::Success);
        CHECK(ClassifyNtStatus(0x00000103L) == AreaStatus::Success); // STATUS_PENDING is a success code
        CHECK(ClassifyNtStatus(kStatusInvalidInfoClass) == AreaStatus::NotSupported);
        CHECK(ClassifyNtStatus(kStatusNotImplemented) == AreaStatus::NotSupported);
        CHECK(ClassifyNtStatus(kStatusNotSupported) == AreaStatus::NotSupported);
        CHECK(ClassifyNtStatus(kStatusPrivilegeNotHeld) == AreaStatus::PrivilegeMissing);
        CHECK(ClassifyNtStatus(kStatusAccessDenied) == AreaStatus::Failed);

        CHECK(ClassifyWin32Error(ERROR_SUCCESS) == AreaStatus::Success);
        CHECK(ClassifyWin32Error(ERROR_NOT_SUPPORTED) == AreaStatus::NotSupported);
        CHECK(ClassifyWin32Error(ERROR_PRIVILEGE_NOT_HELD) == AreaStatus::PrivilegeMissing);
        CHECK(ClassifyWin32Error(ERROR_NOT_ALL_ASSIGNED) == AreaStatus::PrivilegeMissing);
        CHECK(ClassifyWin32Error(ERROR_ACCESS_DENIED) == AreaStatus::Failed);

        CHECK(ComputeFreedBytes(100, 250) == 150);
        CHECK(ComputeFreedBytes(250, 100) == 0);
        CHECK(ComputeFreedBytes(7, 7) == 0);

        CHECK(Percent(0, 0) == 0);
        CHECK(Percent(1, 3) == 33);
        CHECK(Percent(2, 3) == 67);
        CHECK(Percent(5, 5) == 100);
        CHECK(Percent(9, 5) == 100);

        const MemoryStatus s = ComputeMemoryStatus(16000, 4000, 32000, 8000);
        CHECK(s.totalPhysical == 16000);
        CHECK(s.availablePhysical == 4000);
        CHECK(s.usedPhysical == 12000);
        CHECK(s.usedPercent == 75);
        CHECK(s.commitTotal == 24000);
        CHECK(s.commitLimit == 32000);
        CHECK(s.commitPercent == 75);

        const MemoryStatus odd = ComputeMemoryStatus(100, 200, 10, 20);
        CHECK(odd.usedPhysical == 0 && odd.usedPercent == 0);
        CHECK(odd.commitTotal == 0 && odd.commitPercent == 0);
    }

} // namespace

int wmain()
{
    std::printf("MemPurge tests\n");
    TestCommandLine();
    TestFormatBytes();
    TestSettings();
    TestProcessNames();
    TestAreas();
    TestStatusAndMemoryMath();
    std::printf("\n%d checks, %d failures\n", g_checks, g_failures);
    std::printf("%s\n", g_failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED");
    return g_failures == 0 ? 0 : 1;
}
