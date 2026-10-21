// MemPurge - ui/SettingsPage.h
// "Settings" tab. Every change is applied and saved immediately.
#pragma once

#include "HotKeyEdit.h"
#include "PageBase.h"
#include "core/Settings.h"

namespace mp
{
    namespace ui
    {
        class CSettingsPage : public CPageBase
        {
        public:
            explicit CSettingsPage(CMainDlg& owner);

            void LoadFrom(const core::Settings& settings, bool startupRegistered);
            void SetStartupChecked(bool registered);
            void ShowHotkeyWarning(bool show);

        protected:
            BOOL OnInitDialog() override;
            afx_msg void OnNumberChanged();
            afx_msg void OnNumberKillFocus();
            afx_msg void OnCheckChanged(UINT id);
            afx_msg void OnHotkeyEnable();
            afx_msg void OnHotkeyChanged();
            afx_msg void OnRunOnStartup();
            afx_msg void OnReset();
            DECLARE_MESSAGE_MAP()

        private:
            void ApplyToOwner();
            core::Settings ReadControls() const;
            void SetCheck(int id, bool value);
            bool GetCheck(int id) const;

            CSpinButtonCtrl intervalSpin_;
            CSpinButtonCtrl lowMemSpin_;
            CSpinButtonCtrl warningSpin_;
            CSpinButtonCtrl dangerSpin_;
            CHotKeyEdit hotkey_;
        };

    } // namespace ui
} // namespace mp
