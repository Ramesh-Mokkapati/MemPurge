// MemPurge - engine/LowPriorityScope.h
// Lowers thread priority and enters process background mode for its lifetime.
#pragma once

namespace mp
{
    namespace engine
    {
        class LowPriorityScope
        {
        public:
            explicit LowPriorityScope(bool enable);
            ~LowPriorityScope();

            LowPriorityScope(const LowPriorityScope&) = delete;
            LowPriorityScope& operator=(const LowPriorityScope&) = delete;

        private:
            int oldThreadPriority_;
            bool threadChanged_;
            bool backgroundMode_;
        };

    } // namespace engine
} // namespace mp
