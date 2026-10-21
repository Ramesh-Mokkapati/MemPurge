// MemPurge - ui/TrayIcon.h
// Notification-area icon (Shell_NotifyIconW, NOTIFYICON_VERSION_4, fixed id).
#pragma once

#include "TrayLevel.h"

namespace mp
{
    namespace ui
    {
        class TrayIcon
        {
        public:
            TrayIcon() = default;
            ~TrayIcon();
            TrayIcon(const TrayIcon&) = delete;
            TrayIcon& operator=(const TrayIcon&) = delete;

            void Init(HWND owner, UINT callbackMessage, HICON appIcon);

            // Adds (or re-adds after an Explorer restart) the icon.
            bool Add();
            void Remove();

            // Updates icon and tooltip; the icon is regenerated only when the value,
            // colour, mode or DPI changed.
            void Update(bool showUsage, unsigned usedPercent, TrayLevel level, const CString& tooltip);

            void ShowNotification(const CString& title, const CString& text);

        private:
            void FillBase(NOTIFYICONDATAW& nid, UINT flags) const;
            HICON CurrentIcon() const;

            HWND owner_ = nullptr;
            UINT callbackMessage_ = 0;
            HICON appIcon_ = nullptr;
            HICON usageIcon_ = nullptr;
            bool added_ = false;
            bool showUsage_ = false;
            int lastValue_ = -1;
            TrayLevel lastLevel_ = TrayLevel::Normal;
            UINT lastDpi_ = 0;
            CString tooltip_;
        };

    } // namespace ui
} // namespace mp
