// MemPurge - ui/MemoryDC.cpp
#include "framework.h"

#include "MemoryDC.h"

namespace mp
{
    namespace ui
    {
        MemoryDC::MemoryDC(HDC compatible)
            : dc_(::CreateCompatibleDC(compatible))
        {
        }

        MemoryDC::~MemoryDC()
        {
            if (dc_ != nullptr)
            {
                ::DeleteDC(dc_);
            }
        }

        HDC MemoryDC::get() const noexcept
        {
            return dc_;
        }

    } // namespace ui
} // namespace mp
