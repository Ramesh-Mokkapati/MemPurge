// MemPurge - core/Settings.h
// Settings model, defaults and clamp/normalize logic (SPEC section 5.6).
// Pure: no registry access (see SettingsStore.h for persistence).
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace mp
{
    namespace core
    {
        // Hot-key control modifier flags (same values as HOTKEYF_* in commctrl.h).
        constexpr std::uint8_t kHotkeyfShift = 0x01;
        constexpr std::uint8_t kHotkeyfControl = 0x02;
        constexpr std::uint8_t kHotkeyfAlt = 0x04;
        constexpr std::uint8_t kHotkeyfExt = 0x08;

        constexpr std::uint32_t kMaxIntervalHours = 24;
        constexpr std::uint32_t kMaxLowMemoryPercent = 99;
        constexpr std::uint32_t kMinLevelPercent = 1;
        constexpr std::uint32_t kMaxLevelPercent = 100;
        constexpr std::uint32_t kDefaultWarningLevel = 80;
        constexpr std::uint32_t kDefaultDangerLevel = 90;

        // MAKEWORD(vk, mods): low byte = virtual key, high byte = HOTKEYF_* flags.
        constexpr std::uint16_t MakeHotkey(std::uint8_t vk, std::uint8_t mods) noexcept
        {
            return static_cast<std::uint16_t>(vk | (static_cast<std::uint16_t>(mods) << 8));
        }
        constexpr std::uint8_t HotkeyVirtualKey(std::uint16_t hotkey) noexcept
        {
            return static_cast<std::uint8_t>(hotkey & 0xFF);
        }
        constexpr std::uint8_t HotkeyModifiers(std::uint16_t hotkey) noexcept
        {
            return static_cast<std::uint8_t>(hotkey >> 8);
        }

        // Default: Ctrl+Shift+M.
        constexpr std::uint16_t kDefaultHotkey =
            MakeHotkey(static_cast<std::uint8_t>('M'), kHotkeyfControl | kHotkeyfShift);

        // Converts HOTKEYF_* flags to RegisterHotKey MOD_* flags (without MOD_NOREPEAT).
        unsigned int HotkeyfToModFlags(std::uint8_t hotkeyfMods) noexcept;

        struct Settings
        {
            std::uint32_t areas = 0;
            std::uint32_t intervalHours = 0;        // 0..24, 0 = off
            std::uint32_t lowMemoryPercent = 0;     // 0..99, 0 = off
            bool hotkeyEnabled = false;
            std::uint16_t hotkey = kDefaultHotkey;
            bool startMinimized = false;
            bool closeToTray = true;
            bool alwaysOnTop = false;
            bool showNotifications = true;
            bool showVirtualMemory = false;
            bool lowPriority = false;
            bool optimizeOnMiddleClick = false;
            bool showUsageInTray = true;
            std::uint32_t warningLevel = kDefaultWarningLevel;  // 1..100
            std::uint32_t dangerLevel = kDefaultDangerLevel;    // 1..100, >= warning
            std::vector<std::wstring> exclusions;               // normalized names

            bool operator==(const Settings& other) const;
            bool operator!=(const Settings& other) const;
        };

        Settings DefaultSettings();

        // Clamps out-of-range values and normalizes the structure in place:
        // areas (unknown bits dropped, standby precedence), interval 0..24,
        // low-memory 0..99, levels 1..100 with warning <= danger, hotkey with no key
        // -> default, modifier bits limited to HOTKEYF_*, exclusions normalized.
        void NormalizeSettings(Settings& settings);

        // REG_MULTI_SZ helpers. ParseMultiSz accepts data with or without terminators;
        // BuildMultiSz returns the string including the final extra terminator.
        std::vector<std::wstring> ParseMultiSz(const wchar_t* data, std::size_t charCount);
        std::wstring BuildMultiSz(const std::vector<std::wstring>& values);

    } // namespace core
} // namespace mp
