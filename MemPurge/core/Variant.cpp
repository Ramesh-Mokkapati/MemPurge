// MemPurge - core/Variant.cpp
#include "core/Variant.h"

namespace mp
{
    namespace core
    {
        Variant::Variant()
        {
            ::VariantInit(&value_);
        }

        Variant::~Variant()
        {
            ::VariantClear(&value_);
        }

        const VARIANT& Variant::get() const noexcept
        {
            return value_;
        }

    } // namespace core
} // namespace mp
