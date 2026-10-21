// MemPurge - ui/MemPurgeApp.h
#pragma once

#include <cstdint>

#include "SingleInstance.h"

namespace mp
{
    namespace ui
    {
        class CMemPurgeApp : public CWinApp
        {
        public:
            CMemPurgeApp() = default;

            BOOL InitInstance() override;
            int ExitInstance() override;

        private:
            int RunHeadless(std::uint32_t areas);
            void ShowUsage(const CString& error);

            SingleInstance instance_;
            bool guiMode_ = false;
            bool comInitialized_ = false;
            int exitCode_ = 0;
        };

    } // namespace ui
} // namespace mp
