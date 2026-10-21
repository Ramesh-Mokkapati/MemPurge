// MemPurge - core/Bstr.h
// RAII BSTR (SysAllocString / SysFreeString).
#pragma once

#include <windows.h>
#include <oleauto.h>

namespace mp
{
    namespace core
    {
        class Bstr
        {
        public:
            explicit Bstr(const wchar_t* s);
            ~Bstr();

            Bstr(const Bstr&) = delete;
            Bstr& operator=(const Bstr&) = delete;

            BSTR get() const noexcept;

            // False when allocation failed for a non-null source string.
            bool valid(const wchar_t* source) const noexcept;

        private:
            BSTR value_;
        };

    } // namespace core
} // namespace mp
