// MemPurge - ui/ScreenDC.cpp
#include "framework.h"

#include "ScreenDC.h"

namespace mp
{
    namespace ui
    {
        ScreenDC::ScreenDC()
            : dc_(::GetDC(nullptr))
        {
        }

        ScreenDC::~ScreenDC()
        {
            if (dc_ != nullptr)
            {
                ::ReleaseDC(nullptr, dc_);
            }
        }

        HDC ScreenDC::get() const noexcept
        {
            return dc_;
        }

    } // namespace ui
} // namespace mp
