// MemPurge - ui/HotKeyEdit.cpp
#include "framework.h"

#include "HotKeyEdit.h"

namespace mp
{
    namespace ui
    {
        BEGIN_MESSAGE_MAP(CHotKeyEdit, CHotKeyCtrl)
            ON_WM_KEYUP()
            ON_WM_SYSKEYUP()
            ON_WM_KILLFOCUS()
        END_MESSAGE_MAP()

        void CHotKeyEdit::NotifyParent()
        {
            CWnd* parent = GetParent();
            if (parent != nullptr)
            {
                parent->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), EN_CHANGE),
                    reinterpret_cast<LPARAM>(m_hWnd));
            }
        }

        void CHotKeyEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
        {
            CHotKeyCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
            NotifyParent();
        }

        void CHotKeyEdit::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
        {
            CHotKeyCtrl::OnSysKeyUp(nChar, nRepCnt, nFlags);
            NotifyParent();
        }

        void CHotKeyEdit::OnKillFocus(CWnd* newWnd)
        {
            CHotKeyCtrl::OnKillFocus(newWnd);
            NotifyParent();
        }

    } // namespace ui
} // namespace mp
