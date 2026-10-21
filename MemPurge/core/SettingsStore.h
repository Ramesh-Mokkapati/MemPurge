// MemPurge - core/SettingsStore.h
// Persistence of Settings under HKEY_CURRENT_USER\Software\MemPurge.
#pragma once

#include "core/Settings.h"

namespace mp
{
    namespace core
    {
        // Missing values take their defaults; the result is normalized (clamped).
        Settings LoadSettings();

        // Writes every value. Returns false if the key or a value could not be written.
        bool SaveSettings(const Settings& settings);

    } // namespace core
} // namespace mp
