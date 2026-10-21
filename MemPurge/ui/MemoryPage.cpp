// MemPurge - ui/MemoryPage.cpp
#include "framework.h"

#include "MemoryPage.h"

#include "MainDlg.h"
#include "UiText.h"
#include "engine/Areas.h"

namespace mp
{
    namespace ui
    {
        namespace
        {
            int AreaControlId(engine::Area area)
            {
                return IDC_AREA_WORKINGSET + static_cast<int>(engine::AreaIndex(area));
            }

        } // namespace

        BEGIN_MESSAGE_MAP(CMemoryPage, CPageBase)
            ON_CONTROL_RANGE(BN_CLICKED, IDC_AREA_WORKINGSET, IDC_AREA_MODIFIEDFILECACHE, &CMemoryPage::OnAreaClicked)
            ON_BN_CLICKED(IDC_OPTIMIZE, &CMemoryPage::OnOptimize)
            ON_BN_CLICKED(IDC_DETAILS, &CMemoryPage::OnDetails)
        END_MESSAGE_MAP()

        CMemoryPage::CMemoryPage(CMainDlg& owner)
            : CPageBase(IDD_PAGE_MEMORY, owner)
        {
        }

        BOOL CMemoryPage::OnInitDialog()
        {
            CPageBase::OnInitDialog();
            physBar_.SubclassDlgItem(IDC_PHYS_BAR, this);
            commitBar_.SubclassDlgItem(IDC_COMMIT_BAR, this);
            physBar_.SetRange32(0, 100);
            commitBar_.SetRange32(0, 100);
            SetDlgItemText(IDC_STATUS, LoadStr(IDS_READY));
            return TRUE;
        }

        void CMemoryPage::ShowMemory(const engine::MemoryStatus& status, bool showVirtual)
        {
            CString text;
            text.Format(LoadStr(IDS_PHYS_FMT), FormatBytesUi(status.usedPhysical).GetString(),
                FormatBytesUi(status.totalPhysical).GetString(), status.usedPercent);
            CString current;
            GetDlgItemText(IDC_PHYS_TEXT, current);
            if (current != text)
            {
                SetDlgItemText(IDC_PHYS_TEXT, text);
            }

            physBar_.SetPos(static_cast<int>(status.usedPercent));

            const int show = showVirtual ? SW_SHOW : SW_HIDE;
            GetDlgItem(IDC_COMMIT_TEXT)->ShowWindow(show);
            commitBar_.ShowWindow(show);
            if (showVirtual)
            {
                text.Format(LoadStr(IDS_COMMIT_FMT), FormatBytesUi(status.commitTotal).GetString(),
                    FormatBytesUi(status.commitLimit).GetString(), status.commitPercent);
                GetDlgItemText(IDC_COMMIT_TEXT, current);
                if (current != text)
                {
                    SetDlgItemText(IDC_COMMIT_TEXT, text);
                }

                commitBar_.SetPos(static_cast<int>(status.commitPercent));
            }
        }

        void CMemoryPage::SetAreas(std::uint32_t mask)
        {
            for (engine::Area area : engine::kAreaOrder)
            {
                CheckDlgButton(AreaControlId(area), engine::HasArea(mask, area) ? BST_CHECKED : BST_UNCHECKED);
            }

            UpdateOptimizeEnabled();
        }

        std::uint32_t CMemoryPage::ReadAreas() const
        {
            std::uint32_t mask = 0;
            for (engine::Area area : engine::kAreaOrder)
            {
                if (IsDlgButtonChecked(AreaControlId(area)) == BST_CHECKED)
                {
                    mask |= engine::ToMask(area);
                }
            }

            return mask;
        }

        void CMemoryPage::OnAreaClicked(UINT id)
        {
            const int standby = AreaControlId(engine::Area::StandbyList);
            const int standbyLow = AreaControlId(engine::Area::StandbyListLowPriority);
            if (static_cast<int>(id) == standby && IsDlgButtonChecked(standby) == BST_CHECKED)
            {
                CheckDlgButton(standbyLow, BST_UNCHECKED);
            }
            else if (static_cast<int>(id) == standbyLow && IsDlgButtonChecked(standbyLow) == BST_CHECKED)
            {
                CheckDlgButton(standby, BST_UNCHECKED);
            }

            core::Settings s = owner_.GetSettings();
            s.areas = ReadAreas();
            owner_.UpdateSettings(s);
        }

        void CMemoryPage::SetBusy(bool busy)
        {
            busy_ = busy;
            UpdateOptimizeEnabled();
            if (busy)
            {
                GetDlgItem(IDC_DETAILS)->ShowWindow(SW_HIDE);
            }
        }

        void CMemoryPage::UpdateOptimizeEnabled()
        {
            GetDlgItem(IDC_OPTIMIZE)->EnableWindow(!busy_ && ReadAreas() != 0);
        }

        void CMemoryPage::SetStatusText(const CString& text, bool showDetails)
        {
            SetDlgItemText(IDC_STATUS, text);
            GetDlgItem(IDC_DETAILS)->ShowWindow(showDetails ? SW_SHOW : SW_HIDE);
        }

        void CMemoryPage::OnOptimize()
        {
            owner_.RequestOptimize(OptimizeReason::Manual);
        }

        void CMemoryPage::OnDetails()
        {
            owner_.ShowDetails();
        }

    } // namespace ui
} // namespace mp
