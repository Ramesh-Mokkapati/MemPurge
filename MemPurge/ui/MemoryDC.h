// MemPurge - ui/MemoryDC.h
// RAII memory device context (CreateCompatibleDC / DeleteDC).
#pragma once

namespace mp
{
    namespace ui
    {
        class MemoryDC
        {
        public:
            explicit MemoryDC(HDC compatible);
            ~MemoryDC();

            MemoryDC(const MemoryDC&) = delete;
            MemoryDC& operator=(const MemoryDC&) = delete;

            HDC get() const noexcept;

        private:
            HDC dc_;
        };

    } // namespace ui
} // namespace mp
