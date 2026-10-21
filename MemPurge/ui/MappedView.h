// MemPurge - ui/MappedView.h
// RAII for MapViewOfFile over the single-instance window-handle mapping.
#pragma once

#include <cstdint>

namespace mp
{
    namespace ui
    {
        class MappedView
        {
        public:
            MappedView(HANDLE mapping, DWORD access);
            ~MappedView();

            MappedView(const MappedView&) = delete;
            MappedView& operator=(const MappedView&) = delete;

            std::uint64_t* get() const noexcept;

        private:
            void* view_;
        };

    } // namespace ui
} // namespace mp
