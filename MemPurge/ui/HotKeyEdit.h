// MemPurge - ui/HotKeyEdit.h
// Hot-key control that reports every change to its parent as EN_CHANGE
// (after key-up and on focus loss), so the page never misses an edit.
#pragma once

namespace mp
{
    namespace ui
    {
        class CHotKeyEdit : public CHotKeyCtrl
        {
        protected:
            afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
            afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
            afx_msg void OnKillFocus(CWnd* newWnd);
            DECLARE_MESSAGE_MAP()

        private:
            void NotifyParent();
        };

    } // namespace ui
} // namespace mp
