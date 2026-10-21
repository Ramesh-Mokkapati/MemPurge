// MemPurge - ui/SelectGuard.h
// Selects a GDI object into a DC and restores the previous one on destruction.
#pragma once

namespace mp
{
    namespace ui
    {
        class SelectGuard
        {
        public:
            SelectGuard(HDC dc, HGDIOBJ obj);
            ~SelectGuard();

            SelectGuard(const SelectGuard&) = delete;
            SelectGuard& operator=(const SelectGuard&) = delete;

        private:
            HDC dc_;
            HGDIOBJ old_;
        };

    } // namespace ui
} // namespace mp
