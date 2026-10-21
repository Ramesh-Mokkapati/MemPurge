// MemPurge - core/RegistryKey.h
// RAII owner for an open registry key (closed with RegCloseKey).
#pragma once

#include <windows.h>

namespace mp
{
    namespace core
    {
        class RegistryKey
        {
        public:
            RegistryKey();
            ~RegistryKey();

            RegistryKey(const RegistryKey&) = delete;
            RegistryKey& operator=(const RegistryKey&) = delete;

            // Output parameter for RegOpenKeyExW / RegCreateKeyExW.
            HKEY* put() noexcept;
            HKEY get() const noexcept;

        private:
            HKEY key_;
        };

    } // namespace core
} // namespace mp
