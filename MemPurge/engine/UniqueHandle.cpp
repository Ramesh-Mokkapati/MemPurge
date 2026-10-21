// MemPurge - engine/UniqueHandle.cpp
#include "engine/UniqueHandle.h"

namespace mp
{
    namespace engine
    {
        UniqueHandle::UniqueHandle() noexcept
            : handle_(nullptr)
        {
        }

        UniqueHandle::UniqueHandle(HANDLE handle) noexcept
            : handle_(handle)
        {
        }

        UniqueHandle::~UniqueHandle()
        {
            reset();
        }

        UniqueHandle::UniqueHandle(UniqueHandle&& other) noexcept
            : handle_(other.release())
        {
        }

        UniqueHandle& UniqueHandle::operator=(UniqueHandle&& other) noexcept
        {
            if (this != &other)
            {
                reset(other.release());
            }

            return *this;
        }

        HANDLE UniqueHandle::get() const noexcept
        {
            return handle_;
        }

        bool UniqueHandle::valid() const noexcept
        {
            return handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE;
        }

        UniqueHandle::operator bool() const noexcept
        {
            return valid();
        }

        HANDLE UniqueHandle::release() noexcept
        {
            HANDLE h = handle_;
            handle_ = nullptr;
            return h;
        }

        void UniqueHandle::reset(HANDLE handle) noexcept
        {
            if (valid())
            {
                ::CloseHandle(handle_);
            }

            handle_ = handle;
        }

    } // namespace engine
} // namespace mp
