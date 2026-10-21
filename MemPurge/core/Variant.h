// MemPurge - core/Variant.h
// RAII VARIANT (VT_EMPTY by default).
#pragma once

#include <windows.h>
#include <oleauto.h>

namespace mp
{
    namespace core
    {
        class Variant
        {
        public:
            Variant();
            ~Variant();

            Variant(const Variant&) = delete;
            Variant& operator=(const Variant&) = delete;

            const VARIANT& get() const noexcept;

        private:
            VARIANT value_;
        };

    } // namespace core
} // namespace mp
