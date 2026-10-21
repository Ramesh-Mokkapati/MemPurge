// MemPurge - core/SettingsStore.cpp
#include <windows.h>

#include "core/SettingsStore.h"
#include "core/RegistryKey.h"

#include <string>
#include <vector>

namespace mp
{
    namespace core
    {
        namespace
        {
            constexpr const wchar_t* kKeyPath = L"Software\\MemPurge";

            void ReadDword(HKEY key, const wchar_t* name, std::uint32_t& value)
            {
                DWORD data = 0;
                DWORD size = sizeof(data);
                DWORD type = 0;
                if (::RegQueryValueExW(key, name, nullptr, &type, reinterpret_cast<BYTE*>(&data), &size) ==
                    ERROR_SUCCESS &&
                    type == REG_DWORD && size == sizeof(data))
                {
                    value = data;
                }
            }

            void ReadBool(HKEY key, const wchar_t* name, bool& value)
            {
                std::uint32_t v = value ? 1u : 0u;
                ReadDword(key, name, v);
                value = v != 0;
            }

            void ReadMultiSz(HKEY key, const wchar_t* name, std::vector<std::wstring>& values)
            {
                DWORD type = 0;
                DWORD size = 0;
                if (::RegQueryValueExW(key, name, nullptr, &type, nullptr, &size) != ERROR_SUCCESS ||
                    type != REG_MULTI_SZ)
                {
                    return;
                }

                for (int attempt = 0; attempt < 3; ++attempt)
                {
                    std::vector<wchar_t> buffer(size / sizeof(wchar_t) + 2, L'\0');
                    DWORD bytes = static_cast<DWORD>(buffer.size() * sizeof(wchar_t));
                    const LSTATUS st = ::RegQueryValueExW(key, name, nullptr, &type,
                        reinterpret_cast<BYTE*>(buffer.data()), &bytes);
                    if (st == ERROR_MORE_DATA)
                    {
                        size = bytes;
                        continue;
                    }

                    if (st == ERROR_SUCCESS && type == REG_MULTI_SZ)
                    {
                        values = ParseMultiSz(buffer.data(), bytes / sizeof(wchar_t));
                    }

                    return;
                }
            }

            bool WriteDword(HKEY key, const wchar_t* name, std::uint32_t value)
            {
                const DWORD data = value;
                return ::RegSetValueExW(key, name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&data),
                    sizeof(data)) == ERROR_SUCCESS;
            }

            bool WriteBool(HKEY key, const wchar_t* name, bool value)
            {
                return WriteDword(key, name, value ? 1u : 0u);
            }

            bool WriteMultiSz(HKEY key, const wchar_t* name, const std::vector<std::wstring>& values)
            {
                const std::wstring data = BuildMultiSz(values);
                return ::RegSetValueExW(key, name, 0, REG_MULTI_SZ, reinterpret_cast<const BYTE*>(data.data()),
                    static_cast<DWORD>(data.size() * sizeof(wchar_t))) == ERROR_SUCCESS;
            }

        } // namespace

        Settings LoadSettings()
        {
            Settings s = DefaultSettings();
            RegistryKey key;
            if (::RegOpenKeyExW(HKEY_CURRENT_USER, kKeyPath, 0, KEY_QUERY_VALUE, key.put()) != ERROR_SUCCESS)
            {
                return s;
            }

            std::uint32_t hotkey = s.hotkey;
            ReadDword(key.get(), L"Areas", s.areas);
            ReadDword(key.get(), L"IntervalHours", s.intervalHours);
            ReadDword(key.get(), L"LowMemoryPercent", s.lowMemoryPercent);
            ReadBool(key.get(), L"HotkeyEnabled", s.hotkeyEnabled);
            ReadDword(key.get(), L"Hotkey", hotkey);
            ReadBool(key.get(), L"StartMinimized", s.startMinimized);
            ReadBool(key.get(), L"CloseToTray", s.closeToTray);
            ReadBool(key.get(), L"AlwaysOnTop", s.alwaysOnTop);
            ReadBool(key.get(), L"ShowNotifications", s.showNotifications);
            ReadBool(key.get(), L"ShowVirtualMemory", s.showVirtualMemory);
            ReadBool(key.get(), L"LowPriority", s.lowPriority);
            ReadBool(key.get(), L"OptimizeOnMiddleClick", s.optimizeOnMiddleClick);
            ReadBool(key.get(), L"ShowUsageInTray", s.showUsageInTray);
            ReadDword(key.get(), L"WarningLevel", s.warningLevel);
            ReadDword(key.get(), L"DangerLevel", s.dangerLevel);
            ReadMultiSz(key.get(), L"Exclusions", s.exclusions);
            s.hotkey = static_cast<std::uint16_t>(hotkey > 0xFFFF ? 0 : hotkey);
            NormalizeSettings(s);
            return s;
        }

        bool SaveSettings(const Settings& settings)
        {
            Settings s = settings;
            NormalizeSettings(s);
            RegistryKey key;
            if (::RegCreateKeyExW(HKEY_CURRENT_USER, kKeyPath, 0, nullptr, REG_OPTION_NON_VOLATILE,
                KEY_SET_VALUE, nullptr, key.put(), nullptr) != ERROR_SUCCESS)
            {
                return false;
            }

            bool ok = true;
            ok &= WriteDword(key.get(), L"Areas", s.areas);
            ok &= WriteDword(key.get(), L"IntervalHours", s.intervalHours);
            ok &= WriteDword(key.get(), L"LowMemoryPercent", s.lowMemoryPercent);
            ok &= WriteBool(key.get(), L"HotkeyEnabled", s.hotkeyEnabled);
            ok &= WriteDword(key.get(), L"Hotkey", s.hotkey);
            ok &= WriteBool(key.get(), L"StartMinimized", s.startMinimized);
            ok &= WriteBool(key.get(), L"CloseToTray", s.closeToTray);
            ok &= WriteBool(key.get(), L"AlwaysOnTop", s.alwaysOnTop);
            ok &= WriteBool(key.get(), L"ShowNotifications", s.showNotifications);
            ok &= WriteBool(key.get(), L"ShowVirtualMemory", s.showVirtualMemory);
            ok &= WriteBool(key.get(), L"LowPriority", s.lowPriority);
            ok &= WriteBool(key.get(), L"OptimizeOnMiddleClick", s.optimizeOnMiddleClick);
            ok &= WriteBool(key.get(), L"ShowUsageInTray", s.showUsageInTray);
            ok &= WriteDword(key.get(), L"WarningLevel", s.warningLevel);
            ok &= WriteDword(key.get(), L"DangerLevel", s.dangerLevel);
            ok &= WriteMultiSz(key.get(), L"Exclusions", s.exclusions);
            return ok;
        }

    } // namespace core
} // namespace mp
