// MemPurge - ui/ExclusionsPage.h
// "Exclusions" tab: process names skipped by working-set trimming.
#pragma once

#include <string>
#include <vector>

#include "PageBase.h"

namespace mp
{
    namespace ui
    {
        class CExclusionsPage : public CPageBase
        {
        public:
            explicit CExclusionsPage(CMainDlg& owner);

            void SetList(const std::vector<std::wstring>& names);
            void RefreshProcessNames();

        protected:
            BOOL OnInitDialog() override;
            afx_msg void OnAdd();
            afx_msg void OnRemove();
            afx_msg void OnSelChange();
            DECLARE_MESSAGE_MAP()

        private:
            void UpdateButtons();

            CListBox list_;
            CComboBox combo_;
        };

    } // namespace ui
} // namespace mp
