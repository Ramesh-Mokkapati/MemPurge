// MemPurge - ui/SelectGuard.cpp
#include "framework.h"

#include "SelectGuard.h"

namespace mp
{
    namespace ui
    {
        SelectGuard::SelectGuard(HDC dc, HGDIOBJ obj)
            : dc_(dc)
            , old_(::SelectObject(dc, obj))
        {
        }

        SelectGuard::~SelectGuard()
        {
            if (old_ != nullptr && old_ != HGDI_ERROR)
            {
                ::SelectObject(dc_, old_);
            }
        }

    } // namespace ui
} // namespace mp
