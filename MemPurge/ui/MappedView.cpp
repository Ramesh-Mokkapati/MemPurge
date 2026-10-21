// MemPurge - ui/MappedView.cpp
#include "framework.h"

#include "MappedView.h"

namespace mp
{
    namespace ui
    {
        MappedView::MappedView(HANDLE mapping, DWORD access)
            : view_(::MapViewOfFile(mapping, access, 0, 0, sizeof(std::uint64_t)))
        {
        }

        MappedView::~MappedView()
        {
            if (view_ != nullptr)
            {
                ::UnmapViewOfFile(view_);
            }
        }

        std::uint64_t* MappedView::get() const noexcept
        {
            return static_cast<std::uint64_t*>(view_);
        }

    } // namespace ui
} // namespace mp
