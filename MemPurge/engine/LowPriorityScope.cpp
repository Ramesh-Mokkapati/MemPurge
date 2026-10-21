// MemPurge - engine/LowPriorityScope.cpp
#include <windows.h>

#include "engine/LowPriorityScope.h"

namespace mp
{
    namespace engine
    {
        LowPriorityScope::LowPriorityScope(bool enable)
            : oldThreadPriority_(THREAD_PRIORITY_NORMAL)
            , threadChanged_(false)
            , backgroundMode_(false)
        {
            if (!enable)
            {
                return;
            }

            oldThreadPriority_ = ::GetThreadPriority(::GetCurrentThread());
            threadChanged_ = oldThreadPriority_ != THREAD_PRIORITY_ERROR_RETURN &&
                ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
            backgroundMode_ =
                ::SetPriorityClass(::GetCurrentProcess(), PROCESS_MODE_BACKGROUND_BEGIN) != FALSE;
        }

        LowPriorityScope::~LowPriorityScope()
        {
            if (backgroundMode_)
            {
                ::SetPriorityClass(::GetCurrentProcess(), PROCESS_MODE_BACKGROUND_END);
            }

            if (threadChanged_)
            {
                ::SetThreadPriority(::GetCurrentThread(), oldThreadPriority_);
            }
        }

    } // namespace engine
} // namespace mp
