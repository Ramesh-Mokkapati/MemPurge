// MemPurge - ui/SingleInstance.h
// Single-instance guard: named mutex + HWND published in a named file mapping.
#pragma once

#include "engine/UniqueHandle.h"

namespace mp
{
    namespace ui
    {
        class SingleInstance
        {
        public:
            // Creates Local\MemPurge.SingleInstance. Returns false when another
            // instance already owns it.
            bool Acquire();

            // Publishes the main window handle for later instances.
            void PublishWindow(HWND hwnd);

            // Asks the running instance to show itself (registered window message).
            static void ActivateExisting();

            // Registered message the main window handles to show itself.
            static UINT ActivateMessage();

        private:
            engine::UniqueHandle mutex_;
            engine::UniqueHandle mapping_;
        };

    } // namespace ui
} // namespace mp
