// MemPurge - core/Bstr.cpp
#include "core/Bstr.h"

namespace mp
{
    namespace core
    {
        Bstr::Bstr(const wchar_t* s)
            : value_(::SysAllocString(s))
        {
        }

        Bstr::~Bstr()
        {
            ::SysFreeString(value_);
        }

        BSTR Bstr::get() const noexcept
        {
            return value_;
        }

        bool Bstr::valid(const wchar_t* source) const noexcept
        {
            return value_ != nullptr || source == nullptr;
        }

    } // namespace core
} // namespace mp
