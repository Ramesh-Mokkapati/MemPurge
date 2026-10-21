// MemPurge - ui/SettingsPage.cpp
#include "framework.h"

#include "SettingsPage.h"

#include "MainDlg.h"
#include "UiText.h"

namespace mp
{
    namespace ui
    {
        BEGIN_MESSAGE_MAP(CSettingsPage, CPageBase)
            ON_EN_CHANGE(IDC_INTERVAL_EDIT, &CSettingsPage::OnNumberChanged)
            ON_EN_CHANGE(IDC_LOWMEM_EDIT, &CSettingsPage::OnNumberChanged)
            ON_EN_CHANGE(IDC_WARNING_EDIT, &CSettingsPage::OnNumberChanged)
            ON_EN_CHANGE(IDC_DANGER_EDIT, &CSettingsPage::OnNumberChanged)
            ON_EN_KILLFOCUS(IDC_INTERVAL_EDIT, &CSettingsPage::OnNumberKillFocus)
            ON_EN_KILLFOCUS(IDC_LOWMEM_EDIT, &CSettingsPage::OnNumberKillFocus)
            ON_EN_KILLFOCUS(IDC_WARNING_EDIT, &CSettingsPage::OnNumberKillFocus)
            ON_EN_KILLFOCUS(IDC_DANGER_EDIT, &CSettingsPage::OnNumberKillFocus)
            ON_CONTROL_RANGE(BN_CLICKED, IDC_START_MINIMIZED, IDC_TRAY_USAGE, &CSettingsPage::OnCheckChanged)
            ON_BN_CLICKED(IDC_HOTKEY_ENABLE, &CSettingsPage::OnHotkeyEnable)
            ON_EN_CHANGE(IDC_HOTKEY, &CSettingsPage::OnHotkeyChanged)
            ON_BN_CLICKED(IDC_RUN_ON_STARTUP, &CSettingsPage::OnRunOnStartup)
            ON_BN_CLICKED(IDC_RESET, &CSettingsPage::OnReset)
        END_MESSAGE_MAP()

        CSettingsPage::CSettingsPage(CMainDlg& owner)
            : CPageBase(IDD_PAGE_SETTINGS, owner)
        {
        }

        BOOL CSettingsPage::OnInitDialog()
        {
            loading_ = true;
            CPageBase::OnInitDialog();
            intervalSpin_.SubclassDlgItem(IDC_INTERVAL_SPIN, this);
            lowMemSpin_.SubclassDlgItem(IDC_LOWMEM_SPIN, this);
            warningSpin_.SubclassDlgItem(IDC_WARNING_SPIN, this);
            dangerSpin_.SubclassDlgItem(IDC_DANGER_SPIN, this);
            hotkey_.SubclassDlgItem(IDC_HOTKEY, this);

            intervalSpin_.SetRange32(0, static_cast<int>(core::kMaxIntervalHours));
            lowMemSpin_.SetRange32(0, static_cast<int>(core::kMaxLowMemoryPercent));
            warningSpin_.SetRange32(static_cast<int>(core::kMinLevelPercent), static_cast<int>(core::kMaxLevelPercent));
            dangerSpin_.SetRange32(static_cast<int>(core::kMinLevelPercent), static_cast<int>(core::kMaxLevelPercent));
            SendDlgItemMessage(IDC_INTERVAL_EDIT, EM_SETLIMITTEXT, 2, 0);
            SendDlgItemMessage(IDC_LOWMEM_EDIT, EM_SETLIMITTEXT, 2, 0);
            SendDlgItemMessage(IDC_WARNING_EDIT, EM_SETLIMITTEXT, 3, 0);
            SendDlgItemMessage(IDC_DANGER_EDIT, EM_SETLIMITTEXT, 3, 0);
            SetDlgItemText(IDC_HOTKEY_WARNING, LoadStr(IDS_HOTKEY_FAILED));
            loading_ = false;
            return TRUE;
        }

        void CSettingsPage::SetCheck(int id, bool value)
        {
            CheckDlgButton(id, value ? BST_CHECKED : BST_UNCHECKED);
        }

        bool CSettingsPage::GetCheck(int id) const
        {
            return IsDlgButtonChecked(id) == BST_CHECKED;
        }

        void CSettingsPage::LoadFrom(const core::Settings& s, bool startupRegistered)
        {
            loading_ = true;
            SetDlgItemInt(IDC_INTERVAL_EDIT, s.intervalHours, FALSE);
            SetDlgItemInt(IDC_LOWMEM_EDIT, s.lowMemoryPercent, FALSE);
            SetDlgItemInt(IDC_WARNING_EDIT, s.warningLevel, FALSE);
            SetDlgItemInt(IDC_DANGER_EDIT, s.dangerLevel, FALSE);
            SetCheck(IDC_HOTKEY_ENABLE, s.hotkeyEnabled);

            if (::IsWindow(hotkey_.GetSafeHwnd()))
            {
                hotkey_.SetHotKey(core::HotkeyVirtualKey(s.hotkey), core::HotkeyModifiers(s.hotkey));
            }

            SetCheck(IDC_RUN_ON_STARTUP, startupRegistered);
            SetCheck(IDC_START_MINIMIZED, s.startMinimized);
            SetCheck(IDC_CLOSE_TO_TRAY, s.closeToTray);
            SetCheck(IDC_ALWAYS_ON_TOP, s.alwaysOnTop);
            SetCheck(IDC_SHOW_NOTIFICATIONS, s.showNotifications);
            SetCheck(IDC_SHOW_VIRTUAL, s.showVirtualMemory);
            SetCheck(IDC_LOW_PRIORITY, s.lowPriority);
            SetCheck(IDC_MIDDLE_CLICK, s.optimizeOnMiddleClick);
            SetCheck(IDC_TRAY_USAGE, s.showUsageInTray);
            loading_ = false;
        }

        void CSettingsPage::SetStartupChecked(bool registered)
        {
            SetCheck(IDC_RUN_ON_STARTUP, registered);
        }

        void CSettingsPage::ShowHotkeyWarning(bool show)
        {
            GetDlgItem(IDC_HOTKEY_WARNING)->ShowWindow(show ? SW_SHOW : SW_HIDE);
            if (show)
            {
                SetCheck(IDC_HOTKEY_ENABLE, false);
            }
        }

        core::Settings CSettingsPage::ReadControls() const
        {
            core::Settings s = owner_.GetSettings();
            BOOL ok = FALSE;
            UINT v = GetDlgItemInt(IDC_INTERVAL_EDIT, &ok, FALSE);
            if (ok)
            {
                s.intervalHours = v;
            }

            v = GetDlgItemInt(IDC_LOWMEM_EDIT, &ok, FALSE);
            if (ok)
            {
                s.lowMemoryPercent = v;
            }

            v = GetDlgItemInt(IDC_WARNING_EDIT, &ok, FALSE);
            if (ok)
            {
                s.warningLevel = v;
            }

            v = GetDlgItemInt(IDC_DANGER_EDIT, &ok, FALSE);
            if (ok)
            {
                s.dangerLevel = v;
            }

            s.hotkeyEnabled = GetCheck(IDC_HOTKEY_ENABLE);
            WORD vk = 0;
            WORD mods = 0;
            if (::IsWindow(hotkey_.GetSafeHwnd()))
            {
                hotkey_.GetHotKey(vk, mods);
            }

            if (vk != 0)
            {
                s.hotkey = core::MakeHotkey(static_cast<std::uint8_t>(vk), static_cast<std::uint8_t>(mods));
            }

            s.startMinimized = GetCheck(IDC_START_MINIMIZED);
            s.closeToTray = GetCheck(IDC_CLOSE_TO_TRAY);
            s.alwaysOnTop = GetCheck(IDC_ALWAYS_ON_TOP);
            s.showNotifications = GetCheck(IDC_SHOW_NOTIFICATIONS);
            s.showVirtualMemory = GetCheck(IDC_SHOW_VIRTUAL);
            s.lowPriority = GetCheck(IDC_LOW_PRIORITY);
            s.optimizeOnMiddleClick = GetCheck(IDC_MIDDLE_CLICK);
            s.showUsageInTray = GetCheck(IDC_TRAY_USAGE);
            return s;
        }

        void CSettingsPage::ApplyToOwner()
        {
            if (loading_)
            {
                return;
            }

            owner_.UpdateSettings(ReadControls());
        }

        void CSettingsPage::OnNumberChanged()
        {
            ApplyToOwner();
        }

        void CSettingsPage::OnNumberKillFocus()
        {
            if (loading_)
            {
                return;
            }

            // Show the clamped values once the user leaves the field.
            LoadFrom(owner_.GetSettings(), GetCheck(IDC_RUN_ON_STARTUP));
        }

        void CSettingsPage::OnCheckChanged(UINT /*id*/)
        {
            ApplyToOwner();
        }

        void CSettingsPage::OnHotkeyEnable()
        {
            GetDlgItem(IDC_HOTKEY_WARNING)->ShowWindow(SW_HIDE);
            ApplyToOwner();
        }

        void CSettingsPage::OnHotkeyChanged()
        {
            ApplyToOwner();
        }

        void CSettingsPage::OnRunOnStartup()
        {
            const bool registered = owner_.SetRunOnStartup(GetCheck(IDC_RUN_ON_STARTUP));
            SetStartupChecked(registered);
        }

        void CSettingsPage::OnReset()
        {
            owner_.ResetSettings();
        }

    } // namespace ui
} // namespace mp
