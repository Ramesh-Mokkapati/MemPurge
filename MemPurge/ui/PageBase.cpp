// MemPurge - ui/PageBase.cpp
#include "framework.h"

#include "PageBase.h"

namespace mp
{
    namespace ui
    {
        CPageBase::CPageBase(UINT idd, CMainDlg& owner)
            : CDialog(idd)
            , owner_(owner)
        {
        }

        void CPageBase::OnOK()
        {
        }

        void CPageBase::OnCancel()
        {
        }

    } // namespace ui
} // namespace mp
