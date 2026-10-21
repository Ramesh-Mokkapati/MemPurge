// MemPurge - ui/TrayIcon.cpp
#include "framework.h"

#include "TrayIcon.h"

#include "GdiObject.h"
#include "MemoryDC.h"
#include "ScreenDC.h"
#include "SelectGuard.h"

namespace mp
{
    namespace ui
    {
        namespace
        {
            constexpr UINT kTrayIconId = 1;

            COLORREF BackgroundFor(TrayLevel level)
            {
                switch (level)
                {
                case TrayLevel::Normal:
                    return RGB(0x2E, 0x7D, 0x32); // green

                case TrayLevel::Warning:
                    return RGB(0xF5, 0x9E, 0x0B); // amber

                case TrayLevel::Danger:
                    return RGB(0xC6, 0x28, 0x28); // red

                case TrayLevel::Busy:
                    break;
                }

                return RGB(0x75, 0x75, 0x75); // grey
            }

            COLORREF ForegroundFor(TrayLevel level)
            {
                return level == TrayLevel::Warning ? RGB(0, 0, 0) : RGB(0xFF, 0xFF, 0xFF);
            }

            HICON CreateUsageIcon(unsigned value, TrayLevel level, UINT dpi)
            {
                const int cx = ::GetSystemMetricsForDpi(SM_CXSMICON, dpi);
                const int cy = ::GetSystemMetricsForDpi(SM_CYSMICON, dpi);

                const ScreenDC screen;
                const MemoryDC dc(screen.get());
                const GdiObject<HBITMAP> color(::CreateCompatibleBitmap(screen.get(), cx, cy));
                const GdiObject<HBITMAP> mask(::CreateBitmap(cx, cy, 1, 1, nullptr));
                if (dc.get() == nullptr || color.get() == nullptr || mask.get() == nullptr)
                {
                    return nullptr;
                }

                // Mask: all black = fully opaque.
                {
                    const SelectGuard sel(dc.get(), mask.get());
                    ::PatBlt(dc.get(), 0, 0, cx, cy, BLACKNESS);
                }

                {
                    const SelectGuard sel(dc.get(), color.get());
                    RECT rc{ 0, 0, cx, cy };
                    const GdiObject<HBRUSH> brush(::CreateSolidBrush(BackgroundFor(level)));
                    ::FillRect(dc.get(), &rc, brush.get());

                    const GdiObject<HFONT> font(::CreateFontW(-MulDiv(cy, 80, 100), 0, 0, 0, FW_BOLD, FALSE,
                        FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                        DEFAULT_PITCH | FF_SWISS, L"Segoe UI"));
                    const SelectGuard selFont(dc.get(), font.get());
                    ::SetBkMode(dc.get(), TRANSPARENT);
                    ::SetTextColor(dc.get(), ForegroundFor(level));
                    wchar_t text[4] = {};
                    _snwprintf_s(text, _TRUNCATE, L"%u", value > 99 ? 99u : value);
                    ::DrawTextW(dc.get(), text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                }

                ICONINFO info{};
                info.fIcon = TRUE;
                info.hbmMask = mask.get();
                info.hbmColor = color.get();
                return ::CreateIconIndirect(&info); // copies the bitmaps
            }

        } // namespace

        TrayIcon::~TrayIcon()
        {
            Remove();
            if (usageIcon_ != nullptr)
            {
                ::DestroyIcon(usageIcon_);
            }
        }

        void TrayIcon::Init(HWND owner, UINT callbackMessage, HICON appIcon)
        {
            owner_ = owner;
            callbackMessage_ = callbackMessage;
            appIcon_ = appIcon;
        }

        void TrayIcon::FillBase(NOTIFYICONDATAW& nid, UINT flags) const
        {
            nid = {};
            nid.cbSize = sizeof(nid);
            nid.hWnd = owner_;
            nid.uID = kTrayIconId;
            nid.uFlags = flags;
        }

        HICON TrayIcon::CurrentIcon() const
        {
            return (showUsage_ && usageIcon_ != nullptr) ? usageIcon_ : appIcon_;
        }

        bool TrayIcon::Add()
        {
            if (owner_ == nullptr)
            {
                return false;
            }

            NOTIFYICONDATAW nid;
            FillBase(nid, NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP);
            nid.uCallbackMessage = callbackMessage_;
            nid.hIcon = CurrentIcon();
            wcsncpy_s(nid.szTip, tooltip_.GetString(), _TRUNCATE);

            // After an Explorer restart the old registration is gone; ignore failures here.
            added_ = ::Shell_NotifyIconW(NIM_ADD, &nid) != FALSE;
            if (!added_)
            {
                // The icon may still exist (e.g. TaskbarCreated without a real restart).
                added_ = ::Shell_NotifyIconW(NIM_MODIFY, &nid) != FALSE;
            }

            if (added_)
            {
                NOTIFYICONDATAW ver;
                FillBase(ver, 0);
                ver.uVersion = NOTIFYICON_VERSION_4;
                ::Shell_NotifyIconW(NIM_SETVERSION, &ver);
            }

            return added_;
        }

        void TrayIcon::Remove()
        {
            if (!added_)
            {
                return;
            }

            NOTIFYICONDATAW nid;
            FillBase(nid, 0);
            ::Shell_NotifyIconW(NIM_DELETE, &nid);
            added_ = false;
        }

        void TrayIcon::Update(bool showUsage, unsigned usedPercent, TrayLevel level, const CString& tooltip)
        {
            const unsigned value = usedPercent > 99 ? 99u : usedPercent;
            const UINT dpi = ::GetDpiForSystem();
            bool iconChanged = showUsage != showUsage_;
            showUsage_ = showUsage;

            if (showUsage)
            {
                if (usageIcon_ == nullptr || static_cast<int>(value) != lastValue_ || level != lastLevel_ ||
                    dpi != lastDpi_)
                {
                    HICON icon = CreateUsageIcon(value, level, dpi);
                    if (icon != nullptr)
                    {
                        if (usageIcon_ != nullptr)
                        {
                            ::DestroyIcon(usageIcon_);
                        }

                        usageIcon_ = icon;
                        lastValue_ = static_cast<int>(value);
                        lastLevel_ = level;
                        lastDpi_ = dpi;
                        iconChanged = true;
                    }
                }
            }

            const bool tipChanged = tooltip != tooltip_;
            tooltip_ = tooltip;
            if (!added_ || (!iconChanged && !tipChanged))
            {
                return;
            }

            NOTIFYICONDATAW nid;
            FillBase(nid, (iconChanged ? NIF_ICON : 0u) | (tipChanged ? NIF_TIP : 0u) | NIF_SHOWTIP);
            nid.hIcon = CurrentIcon();
            wcsncpy_s(nid.szTip, tooltip_.GetString(), _TRUNCATE);
            ::Shell_NotifyIconW(NIM_MODIFY, &nid);
        }

        void TrayIcon::ShowNotification(const CString& title, const CString& text)
        {
            if (!added_)
            {
                return;
            }

            NOTIFYICONDATAW nid;
            FillBase(nid, NIF_INFO | NIF_SHOWTIP);
            wcsncpy_s(nid.szInfoTitle, title.GetString(), _TRUNCATE);
            wcsncpy_s(nid.szInfo, text.GetString(), _TRUNCATE);
            nid.dwInfoFlags = NIIF_INFO;
            ::Shell_NotifyIconW(NIM_MODIFY, &nid);
        }

    } // namespace ui
} // namespace mp
