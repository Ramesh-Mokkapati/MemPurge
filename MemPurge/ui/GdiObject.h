// MemPurge - ui/GdiObject.h
// RAII owner for a GDI object (DeleteObject). A class template, so it is
// defined entirely in this header; there is no GdiObject.cpp.
#pragma once

namespace mp
{
    namespace ui
    {
        template <typename T>
        class GdiObject
        {
        public:
            explicit GdiObject(T obj) : obj_(obj)
            {
            }

            ~GdiObject()
            {
                if (obj_ != nullptr)
                {
                    ::DeleteObject(obj_);
                }
            }

            GdiObject(const GdiObject&) = delete;
            GdiObject& operator=(const GdiObject&) = delete;

            T get() const noexcept
            {
                return obj_;
            }

        private:
            T obj_;
        };

    } // namespace ui
} // namespace mp
