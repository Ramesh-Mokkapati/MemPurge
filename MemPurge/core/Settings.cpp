// MemPurge - core/Settings.cpp
#include <windows.h>

#include "core/Settings.h"

#include "engine/Areas.h"
#include "engine/ProcessName.h"

namespace mp
{
    namespace core
    {
        namespace
        {
            std::uint32_t Clamp(std::uint32_t value, std::uint32_t lo, std::uint32_t hi) noexcept
            {
                return value < lo ? lo : (value > hi ? hi : value);
            }
        } // namespace

        unsigned int HotkeyfToModFlags(std::uint8_t hotkeyfMods) noexcept
        {
            unsigned int mods = 0;
            if ((hotkeyfMods & kHotkeyfShift) != 0)
            {
                mods |= MOD_SHIFT;
            }

            if ((hotkeyfMods & kHotkeyfControl) != 0)
            {
                mods |= MOD_CONTROL;
            }

            if ((hotkeyfMods & kHotkeyfAlt) != 0)
            {
                mods |= MOD_ALT;
            }

            return mods;
        }

        bool Settings::operator==(const Settings& other) const
        {
            return areas == other.areas &&
                intervalHours == other.intervalHours &&
                lowMemoryPercent == other.lowMemoryPercent &&
                hotkeyEnabled == other.hotkeyEnabled &&
                hotkey == other.hotkey &&
                startMinimized == other.startMinimized &&
                closeToTray == other.closeToTray &&
                alwaysOnTop == other.alwaysOnTop &&
                showNotifications == other.showNotifications &&
                showVirtualMemory == other.showVirtualMemory &&
                lowPriority == other.lowPriority &&
                optimizeOnMiddleClick == other.optimizeOnMiddleClick &&
                showUsageInTray == other.showUsageInTray &&
                warningLevel == other.warningLevel &&
                dangerLevel == other.dangerLevel &&
                exclusions == other.exclusions;
        }

        bool Settings::operator!=(const Settings& other) const
        {
            return !(*this == other);
        }

        Settings DefaultSettings()
        {
            Settings s;
            s.areas = engine::kAllAreasMask & ~engine::ToMask(engine::Area::StandbyListLowPriority);
            s.intervalHours = 0;
            s.lowMemoryPercent = 0;
            s.hotkeyEnabled = false;
            s.hotkey = kDefaultHotkey;
            s.startMinimized = false;
            s.closeToTray = true;
            s.alwaysOnTop = false;
            s.showNotifications = true;
            s.showVirtualMemory = false;
            s.lowPriority = false;
            s.optimizeOnMiddleClick = false;
            s.showUsageInTray = true;
            s.warningLevel = kDefaultWarningLevel;
            s.dangerLevel = kDefaultDangerLevel;
            s.exclusions.clear();
            return s;
        }

        void NormalizeSettings(Settings& s)
        {
            s.areas = engine::NormalizeAreaMask(s.areas);
            s.intervalHours = Clamp(s.intervalHours, 0, kMaxIntervalHours);
            s.lowMemoryPercent = Clamp(s.lowMemoryPercent, 0, kMaxLowMemoryPercent);
            s.warningLevel = Clamp(s.warningLevel, kMinLevelPercent, kMaxLevelPercent);
            s.dangerLevel = Clamp(s.dangerLevel, kMinLevelPercent, kMaxLevelPercent);

            if (s.warningLevel > s.dangerLevel)
            {
                s.warningLevel = s.dangerLevel;
            }

            if (HotkeyVirtualKey(s.hotkey) == 0)
            {
                s.hotkey = kDefaultHotkey;
            }
            else
            {
                const auto mods = static_cast<std::uint8_t>(
                    HotkeyModifiers(s.hotkey) & (kHotkeyfShift | kHotkeyfControl | kHotkeyfAlt | kHotkeyfExt));
                s.hotkey = MakeHotkey(HotkeyVirtualKey(s.hotkey), mods);
            }

            s.exclusions = engine::NormalizeNameList(s.exclusions);
        }

        std::vector<std::wstring> ParseMultiSz(const wchar_t* data, std::size_t charCount)
        {
            std::vector<std::wstring> values;
            if (data == nullptr)
            {
                return values;
            }

            std::size_t start = 0;
            for (std::size_t i = 0; i <= charCount; ++i)
            {
                if (i == charCount || data[i] == L'\0')
                {
                    if (i > start)
                    {
                        values.emplace_back(data + start, i - start);
                    }
                    else if (i < charCount)
                    {
                        break; // empty string = list terminator
                    }

                    start = i + 1;
                }
            }

            return values;
        }

        std::wstring BuildMultiSz(const std::vector<std::wstring>& values)
        {
            std::wstring out;
            for (const auto& v : values)
            {
                if (v.empty())
                {
                    continue;
                }

                out += v;
                out.push_back(L'\0');
            }

            if (out.empty())
            {
                out.push_back(L'\0');
            }

            out.push_back(L'\0');
            return out;
        }

    } // namespace core
} // namespace mp
