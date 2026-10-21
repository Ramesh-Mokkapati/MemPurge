// MemPurge - ui/MainDlg.h
// Main window (modeless dialog) that owns the tray icon, timers, hotkey and
// the worker thread that runs the engine.
#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include "ExclusionsPage.h"
#include "MemoryPage.h"
#include "SettingsPage.h"
#include "SingleInstance.h"
#include "TrayIcon.h"
#include "UiText.h"
#include "core/Settings.h"
#include "engine/MemoryStatus.h"
#include "engine/Optimizer.h"

namespace mp
{
    namespace ui
    {
        class CMainDlg : public CDialog
        {
        public:
            CMainDlg(bool startHidden, SingleInstance& instance);
            ~CMainDlg() override;

            BOOL CreateModeless();

            const core::Settings& GetSettings() const noexcept
            {
                return settings_;
            }

            // Normalizes, saves and applies a new settings set (timers, hotkey, tray, ...).
            void UpdateSettings(const core::Settings& settings);

            // Settings > Reset to defaults (keeps the exclusion list).
            void ResetSettings();

            // Single entry point for every optimization trigger. Ignored (returns
            // false) while a run is active or when no area is selected.
            bool RequestOptimize(OptimizeReason reason);
            bool IsOptimizing() const noexcept
            {
                return running_;
            }

            // Creates/deletes the startup task; returns whether it now exists.
            bool SetRunOnStartup(bool enable);

            void ShowDetails();

        protected:
            BOOL OnInitDialog() override;
            void OnOK() override {}
            void OnCancel() override {}
            void PostNcDestroy() override;

            afx_msg void OnSysCommand(UINT id, LPARAM lParam);
            afx_msg void OnClose();
            afx_msg void OnDestroy();
            afx_msg void OnTimer(UINT_PTR id);
            afx_msg void OnTabChanged(NMHDR* hdr, LRESULT* result);
            afx_msg LRESULT OnHotKeyMsg(WPARAM wParam, LPARAM lParam);
            afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);
            afx_msg LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
            afx_msg LRESULT OnActivateRequest(WPARAM wParam, LPARAM lParam);
            afx_msg LRESULT OnRunProgress(WPARAM wParam, LPARAM lParam);
            afx_msg LRESULT OnRunDone(WPARAM wParam, LPARAM lParam);
            afx_msg LRESULT OnPowerBroadcastMsg(WPARAM wParam, LPARAM lParam);
            afx_msg void OnTrayOptimize();
            afx_msg void OnTrayShow();
            afx_msg void OnTrayExit();
            DECLARE_MESSAGE_MAP()

        private:
            void CreatePages();
            void SelectPage(int index);
            void RefreshMemory();
            void UpdateTray();
            void RestartTimers();
            void RestartIntervalTimer();
            void CheckLowMemory();
            bool ApplyHotkey();
            void ApplyTopmost();
            void ApplyAll(const core::Settings& old, bool force);
            void ShowMainWindow();
            void HideToTray();
            void ToggleWindow();
            void ShowTrayMenu(int x, int y);
            void JoinWorker();

            bool startHidden_;
            SingleInstance& instance_;
            core::Settings settings_;
            engine::MemoryStatus memory_{};
            TrayIcon tray_;
            HICON appIconLarge_ = nullptr;
            HICON appIconSmall_ = nullptr;
            bool hotkeyRegistered_ = false;
            bool saveErrorShown_ = false;

            CTabCtrl tab_;
            std::unique_ptr<CMemoryPage> memoryPage_;
            std::unique_ptr<CSettingsPage> settingsPage_;
            std::unique_ptr<CExclusionsPage> exclusionsPage_;

            // Worker / run state
            std::thread worker_;
            std::atomic<bool> running_{ false };
            std::mutex resultMutex_;
            engine::RunResult pendingResult_;
            engine::RunResult lastResult_;
            OptimizeReason currentReason_ = OptimizeReason::Manual;
            ULONGLONG lastLowMemoryRunTick_ = 0;
            bool lowMemoryRunHappened_ = false;
        };

    } // namespace ui
} // namespace mp
