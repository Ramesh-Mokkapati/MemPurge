// MemPurge - ui/PageBase.h
// Base for the tab pages: Enter/Esc do nothing, parent is the main dialog.
#pragma once

namespace mp
{
    namespace ui
    {
        class CMainDlg;

        class CPageBase : public CDialog
        {
        public:
            CPageBase(UINT idd, CMainDlg& owner);

        protected:
            void OnOK() override;
            void OnCancel() override;

            CMainDlg& owner_;
            // True until OnInitDialog finishes: auto-buddy spin controls send EN_CHANGE while the
            // dialog is still being created, before member controls are subclassed.
            bool loading_ = true;
        };

    } // namespace ui
} // namespace mp
