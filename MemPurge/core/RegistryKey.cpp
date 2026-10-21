// MemPurge - core/RegistryKey.cpp
#include "core/RegistryKey.h"

namespace mp
{
    namespace core
    {
        RegistryKey::RegistryKey()
            : key_(nullptr)
        {
        }

        RegistryKey::~RegistryKey()
        {
            if (key_ != nullptr)
            {
                ::RegCloseKey(key_);
            }
        }

        HKEY* RegistryKey::put() noexcept
        {
            return &key_;
        }

        HKEY RegistryKey::get() const noexcept
        {
            return key_;
        }

    } // namespace core
} // namespace mp
