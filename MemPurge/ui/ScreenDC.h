// MemPurge - ui/ScreenDC.h
// RAII screen device context (GetDC(nullptr) / ReleaseDC).
#pragma once

namespace mp
{
    namespace ui
    {
        class ScreenDC
        {
        public:
            ScreenDC();
            ~ScreenDC();

            ScreenDC(const ScreenDC&) = delete;
            ScreenDC& operator=(const ScreenDC&) = delete;

            HDC get() const noexcept;

        private:
            HDC dc_;
        };

    } // namespace ui
} // namespace mp
