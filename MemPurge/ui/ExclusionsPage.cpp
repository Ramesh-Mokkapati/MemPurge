// MemPurge - ui/ExclusionsPage.cpp
#include "framework.h"

#include "ExclusionsPage.h"

#include <algorithm>

#include "MainDlg.h"
#include "engine/ProcessName.h"
#include "engine/Processes.h"

namespace mp
{
    namespace ui
    {
        BEGIN_MESSAGE_MAP(CExclusionsPage, CPageBase)
            ON_BN_CLICKED(IDC_EXCL_ADD, &CExclusionsPage::OnAdd)
            ON_BN_CLICKED(IDC_EXCL_REMOVE, &CExclusionsPage::OnRemove)
            ON_LBN_SELCHANGE(IDC_EXCL_LIST, &CExclusionsPage::OnSelChange)
        END_MESSAGE_MAP()

        CExclusionsPage::CExclusionsPage(CMainDlg& owner)
            : CPageBase(IDD_PAGE_EXCLUSIONS, owner)
        {
        }

        BOOL CExclusionsPage::OnInitDialog()
        {
            CPageBase::OnInitDialog();
            list_.SubclassDlgItem(IDC_EXCL_LIST, this);
            combo_.SubclassDlgItem(IDC_EXCL_COMBO, this);
            SetList(owner_.GetSettings().exclusions);
            RefreshProcessNames();
            return TRUE;
        }

        void CExclusionsPage::SetList(const std::vector<std::wstring>& names)
        {
            list_.SetRedraw(FALSE);
            list_.ResetContent();
            for (const auto& name : names)
            {
                list_.AddString(name.c_str());
            }

            list_.SetRedraw(TRUE);
            list_.Invalidate();
            UpdateButtons();
        }

        void CExclusionsPage::RefreshProcessNames()
        {
            CString text;
            combo_.GetWindowText(text);
            combo_.SetRedraw(FALSE);
            combo_.ResetContent();

            for (const auto& name : engine::EnumerateProcessNames())
            {
                combo_.AddString(name.c_str());
            }

            combo_.SetRedraw(TRUE);
            combo_.SetWindowText(text);
        }

        void CExclusionsPage::UpdateButtons()
        {
            GetDlgItem(IDC_EXCL_REMOVE)->EnableWindow(list_.GetCurSel() != LB_ERR);
        }

        void CExclusionsPage::OnAdd()
        {
            CString text;
            combo_.GetWindowText(text);

            const std::wstring name = engine::NormalizeProcessName(text.GetString());
            if (name.empty())
            {
                return;
            }

            core::Settings s = owner_.GetSettings();
            s.exclusions.push_back(name);
            owner_.UpdateSettings(s);
            combo_.SetWindowText(L"");

            const int index = list_.FindStringExact(-1, name.c_str());
            if (index != LB_ERR)
            {
                list_.SetCurSel(index);
            }

            UpdateButtons();
        }

        void CExclusionsPage::OnRemove()
        {
            const int sel = list_.GetCurSel();
            if (sel == LB_ERR)
            {
                return;
            }

            CString text;
            list_.GetText(sel, text);
            core::Settings s = owner_.GetSettings();
            const std::wstring name(text.GetString());
            s.exclusions.erase(std::remove(s.exclusions.begin(), s.exclusions.end(), name), s.exclusions.end());
            owner_.UpdateSettings(s);

            const int count = list_.GetCount();
            if (count > 0)
            {
                list_.SetCurSel(sel < count ? sel : count - 1);
            }

            UpdateButtons();
        }

        void CExclusionsPage::OnSelChange()
        {
            UpdateButtons();
        }

    } // namespace ui
} // namespace mp
