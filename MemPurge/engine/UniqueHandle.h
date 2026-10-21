// MemPurge - engine/UniqueHandle.h
// RAII owner for kernel HANDLEs (closed with CloseHandle).
#pragma once

#include <windows.h>

namespace mp
{
    namespace engine
    {
        class UniqueHandle
        {
        public:
            UniqueHandle() noexcept;
            explicit UniqueHandle(HANDLE handle) noexcept;
            ~UniqueHandle();

            UniqueHandle(const UniqueHandle&) = delete;
            UniqueHandle& operator=(const UniqueHandle&) = delete;

            UniqueHandle(UniqueHandle&& other) noexcept;
            UniqueHandle& operator=(UniqueHandle&& other) noexcept;

            HANDLE get() const noexcept;
            bool valid() const noexcept;
            explicit operator bool() const noexcept;

            HANDLE release() noexcept;
            void reset(HANDLE handle = nullptr) noexcept;

        private:
            HANDLE handle_;
        };

    } // namespace engine
} // namespace mp
