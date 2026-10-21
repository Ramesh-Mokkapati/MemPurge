// MemPurge - ui/MemoryPage.h
// "Memory" tab: usage display, area checkboxes, Optimize button and status.
#pragma once

#include <cstdint>

#include "PageBase.h"
#include "engine/MemoryStatus.h"

namespace mp
{
    namespace ui
    {
        class CMemoryPage : public CPageBase
        {
        public:
            explicit CMemoryPage(CMainDlg& owner);

            void ShowMemory(const engine::MemoryStatus& status, bool showVirtual);
            void SetAreas(std::uint32_t mask);
            void SetBusy(bool busy);
            void SetStatusText(const CString& text, bool showDetails);

        protected:
            BOOL OnInitDialog() override;
            afx_msg void OnAreaClicked(UINT id);
            afx_msg void OnOptimize();
            afx_msg void OnDetails();
            DECLARE_MESSAGE_MAP()

        private:
            std::uint32_t ReadAreas() const;
            void UpdateOptimizeEnabled();

            CProgressCtrl physBar_;
            CProgressCtrl commitBar_;
            bool busy_ = false;
        };

    } // namespace ui
} // namespace mp
