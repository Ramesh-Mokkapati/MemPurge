// MemPurge - ui/MainDlg.cpp
#include "framework.h"

#include "MainDlg.h"

#include <system_error>
#include <uxtheme.h>
#include <vector>

#include "core/SettingsStore.h"
#include "core/StartupTask.h"
#include "engine/Areas.h"

namespace mp
{
    namespace ui
    {
        namespace
        {
            constexpr UINT WM_APP_TRAY = WM_APP + 1;
            constexpr UINT WM_APP_PROGRESS = WM_APP + 2;
            constexpr UINT WM_APP_DONE = WM_APP + 3;

            constexpr UINT_PTR kTimerRefresh = 1;
            constexpr UINT_PTR kTimerLowMemory = 2;
            constexpr UINT_PTR kTimerInterval = 3;
            constexpr UINT kRefreshMs = 1000;
            constexpr UINT kLowMemoryCheckMs = 5000;
            constexpr ULONGLONG kLowMemoryMinGapMs = 5ull * 60ull * 1000ull;
            constexpr UINT kMsPerHour = 60u * 60u * 1000u;

            constexpr int kHotkeyId = 1;

            const UINT kTaskbarCreatedMsg = ::RegisterWindowMessageW(L"TaskbarCreated");
            const UINT kActivateMsg = SingleInstance::ActivateMessage();

            CString ExecutablePath()
            {
                std::vector<wchar_t> buffer(MAX_PATH);
                for (;;)
                {
                    const DWORD len = ::GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
                    if (len == 0)
                    {
                        return CString();
                    }

                    if (len < buffer.size())
                    {
                        return CString(buffer.data(), static_cast<int>(len));
                    }

                    buffer.resize(buffer.size() * 2);
                }
            }

            HICON LoadAppIcon(bool large)
            {
                HICON icon = nullptr;
                if (FAILED(::LoadIconMetric(AfxGetResourceHandle(), MAKEINTRESOURCEW(IDR_MAINFRAME),
                    large ? LIM_LARGE : LIM_SMALL, &icon)))
                {
                    return nullptr;
                }

                return icon;
            }

        } // namespace

        BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
            ON_WM_SYSCOMMAND()
            ON_WM_CLOSE()
            ON_WM_DESTROY()
            ON_WM_TIMER()
            ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CMainDlg::OnTabChanged)
            ON_MESSAGE(WM_HOTKEY, &CMainDlg::OnHotKeyMsg)
            ON_MESSAGE(WM_APP_TRAY, &CMainDlg::OnTrayNotify)
            ON_MESSAGE(WM_APP_PROGRESS, &CMainDlg::OnRunProgress)
            ON_MESSAGE(WM_APP_DONE, &CMainDlg::OnRunDone)
            ON_MESSAGE(WM_POWERBROADCAST, &CMainDlg::OnPowerBroadcastMsg)
            ON_REGISTERED_MESSAGE(kTaskbarCreatedMsg, &CMainDlg::OnTaskbarCreated)
            ON_REGISTERED_MESSAGE(kActivateMsg, &CMainDlg::OnActivateRequest)
            ON_COMMAND(ID_TRAY_OPTIMIZE, &CMainDlg::OnTrayOptimize)
            ON_COMMAND(ID_TRAY_SHOW, &CMainDlg::OnTrayShow)
            ON_COMMAND(ID_TRAY_EXIT, &CMainDlg::OnTrayExit)
        END_MESSAGE_MAP()

        CMainDlg::CMainDlg(bool startHidden, SingleInstance& instance)
            : CDialog(IDD_MAIN)
            , startHidden_(startHidden)
            , instance_(instance)
        {
        }

        CMainDlg::~CMainDlg()
        {
            JoinWorker();
            if (appIconLarge_ != nullptr)
            {
                ::DestroyIcon(appIconLarge_);
            }

            if (appIconSmall_ != nullptr)
            {
                ::DestroyIcon(appIconSmall_);
            }
        }

        BOOL CMainDlg::CreateModeless()
        {
            if (!Create(IDD_MAIN, nullptr))
            {
                return FALSE;
            }

            if (!startHidden_)
            {
                ShowMainWindow();
            }

            return TRUE;
        }

        BOOL CMainDlg::OnInitDialog()
        {
            CDialog::OnInitDialog();

            settings_ = core::LoadSettings();

            appIconLarge_ = LoadAppIcon(true);
            appIconSmall_ = LoadAppIcon(false);
            SetIcon(appIconLarge_, TRUE);
            SetIcon(appIconSmall_, FALSE);
            SetWindowText(LoadStr(IDS_APP_TITLE));

            // The process is elevated; allow Explorer (medium integrity) to deliver
            // TaskbarCreated and the tray callback message.
            ::ChangeWindowMessageFilterEx(m_hWnd, kTaskbarCreatedMsg, MSGFLT_ALLOW, nullptr);
            ::ChangeWindowMessageFilterEx(m_hWnd, WM_APP_TRAY, MSGFLT_ALLOW, nullptr);

            tab_.SubclassDlgItem(IDC_TAB, this);
            tab_.InsertItem(0, LoadStr(IDS_TAB_MEMORY));
            tab_.InsertItem(1, LoadStr(IDS_TAB_SETTINGS));
            tab_.InsertItem(2, LoadStr(IDS_TAB_EXCLUSIONS));
            CreatePages();
            SelectPage(0);

            tray_.Init(m_hWnd, WM_APP_TRAY, appIconSmall_);
            RefreshMemory();
            tray_.Add();

            ApplyAll(settings_, true);
            instance_.PublishWindow(m_hWnd);
            return TRUE;
        }

        void CMainDlg::CreatePages()
        {
            memoryPage_.reset(new CMemoryPage(*this));
            settingsPage_.reset(new CSettingsPage(*this));
            exclusionsPage_.reset(new CExclusionsPage(*this));

            CRect rc;
            tab_.GetWindowRect(&rc);
            ScreenToClient(&rc);
            tab_.AdjustRect(FALSE, &rc);

            CDialog* pages[] = { memoryPage_.get(), settingsPage_.get(), exclusionsPage_.get() };
            const UINT ids[] = { IDD_PAGE_MEMORY, IDD_PAGE_SETTINGS, IDD_PAGE_EXCLUSIONS };
            for (int i = 0; i < 3; ++i)
            {
                pages[i]->Create(ids[i], this);
                ::EnableThemeDialogTexture(pages[i]->GetSafeHwnd(), ETDT_ENABLETAB);
                pages[i]->SetWindowPos(&CWnd::wndTop, rc.left, rc.top, rc.Width(), rc.Height(),
                    SWP_NOACTIVATE | SWP_HIDEWINDOW);
            }
        }

        void CMainDlg::SelectPage(int index)
        {
            CDialog* pages[] = { memoryPage_.get(), settingsPage_.get(), exclusionsPage_.get() };
            for (int i = 0; i < 3; ++i)
            {
                if (i != index)
                {
                    pages[i]->ShowWindow(SW_HIDE);
                }
            }

            if (index == 2)
            {
                exclusionsPage_->RefreshProcessNames();
            }

            if (index >= 0 && index < 3)
            {
                pages[index]->ShowWindow(SW_SHOW);
            }
        }

        void CMainDlg::OnTabChanged(NMHDR* /*hdr*/, LRESULT* result)
        {
            SelectPage(tab_.GetCurSel());
            *result = 0;
        }

        // ---- Settings ---------------------------------------------------------------

        void CMainDlg::UpdateSettings(const core::Settings& settings)
        {
            const core::Settings old = settings_;
            settings_ = settings;
            core::NormalizeSettings(settings_);
            if (!core::SaveSettings(settings_) && !saveErrorShown_)
            {
                saveErrorShown_ = true;
                MessageBox(LoadStr(IDS_SETTINGS_SAVE_FAILED), LoadStr(IDS_APP_TITLE), MB_OK | MB_ICONWARNING);
            }

            ApplyAll(old, false);
        }

        void CMainDlg::ResetSettings()
        {
            core::Settings s = core::DefaultSettings();
            s.exclusions = settings_.exclusions;
            UpdateSettings(s);
            settingsPage_->LoadFrom(settings_, core::IsStartupTaskRegistered());
            if (!settings_.hotkeyEnabled)
            {
                settingsPage_->ShowHotkeyWarning(false);
            }
        }

        void CMainDlg::ApplyAll(const core::Settings& old, bool force)
        {
            if (force || old.hotkeyEnabled != settings_.hotkeyEnabled || old.hotkey != settings_.hotkey)
            {
                ApplyHotkey();
            }

            if (force || old.alwaysOnTop != settings_.alwaysOnTop)
            {
                ApplyTopmost();
            }

            if (force)
            {
                RestartTimers();
            }
            else if (old.intervalHours != settings_.intervalHours)
            {
                RestartIntervalTimer();
            }

            memoryPage_->SetAreas(settings_.areas);
            if (force || old.exclusions != settings_.exclusions)
            {
                exclusionsPage_->SetList(settings_.exclusions);
            }

            if (force)
            {
                settingsPage_->LoadFrom(settings_, core::IsStartupTaskRegistered());
            }

            RefreshMemory();
        }

        bool CMainDlg::ApplyHotkey()
        {
            if (hotkeyRegistered_)
            {
                ::UnregisterHotKey(m_hWnd, kHotkeyId);
                hotkeyRegistered_ = false;
            }

            if (!settings_.hotkeyEnabled)
            {
                return true;
            }

            const UINT mods = core::HotkeyfToModFlags(core::HotkeyModifiers(settings_.hotkey)) | MOD_NOREPEAT;
            const UINT vk = core::HotkeyVirtualKey(settings_.hotkey);
            if (::RegisterHotKey(m_hWnd, kHotkeyId, mods, vk))
            {
                hotkeyRegistered_ = true;
                settingsPage_->ShowHotkeyWarning(false);
                return true;
            }

            // Registration failed: inline warning, checkbox stays unchecked.
            settings_.hotkeyEnabled = false;
            core::SaveSettings(settings_);
            settingsPage_->ShowHotkeyWarning(true);
            return false;
        }

        void CMainDlg::ApplyTopmost()
        {
            SetWindowPos(settings_.alwaysOnTop ? &CWnd::wndTopMost : &CWnd::wndNoTopMost, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }

        bool CMainDlg::SetRunOnStartup(bool enable)
        {
            const HRESULT hr = enable ? core::RegisterStartupTask(ExecutablePath().GetString())
                : core::DeleteStartupTask();
            if (FAILED(hr))
            {
                CString msg;
                msg.Format(LoadStr(IDS_STARTUP_FAILED_FMT), static_cast<unsigned>(hr),
                    engine::SystemErrorMessage(static_cast<unsigned long>(hr)).c_str());
                MessageBox(msg, LoadStr(IDS_APP_TITLE), MB_OK | MB_ICONWARNING);
            }

            return core::IsStartupTaskRegistered();
        }

        // ---- Memory display / tray ------------------------------------------------------

        void CMainDlg::RefreshMemory()
        {
            engine::MemoryStatus status;
            if (engine::QueryMemoryStatus(status))
            {
                memory_ = status;
            }

            if (memoryPage_ && memoryPage_->GetSafeHwnd() != nullptr)
            {
                memoryPage_->ShowMemory(memory_, settings_.showVirtualMemory);
            }

            UpdateTray();
        }

        void CMainDlg::UpdateTray()
        {
            TrayLevel level = TrayLevel::Normal;
            if (running_)
            {
                level = TrayLevel::Busy;
            }
            else if (memory_.usedPercent >= settings_.dangerLevel)
            {
                level = TrayLevel::Danger;
            }
            else if (memory_.usedPercent >= settings_.warningLevel)
            {
                level = TrayLevel::Warning;
            }

            CString tip;
            tip.Format(LoadStr(IDS_TOOLTIP_FMT), memory_.usedPercent);
            tray_.Update(settings_.showUsageInTray, memory_.usedPercent, level, tip);
        }

        // ---- Timers ---------------------------------------------------------------------

        void CMainDlg::RestartTimers()
        {
            KillTimer(kTimerRefresh);
            KillTimer(kTimerLowMemory);
            SetTimer(kTimerRefresh, kRefreshMs, nullptr);
            SetTimer(kTimerLowMemory, kLowMemoryCheckMs, nullptr);
            RestartIntervalTimer();
        }

        void CMainDlg::RestartIntervalTimer()
        {
            KillTimer(kTimerInterval);
            if (settings_.intervalHours > 0)
            {
                SetTimer(kTimerInterval, settings_.intervalHours * kMsPerHour, nullptr);
            }
        }

        void CMainDlg::OnTimer(UINT_PTR id)
        {
            switch (id)
            {
            case kTimerRefresh:
                RefreshMemory();
                break;

            case kTimerLowMemory:
                CheckLowMemory();
                break;

            case kTimerInterval:
                RequestOptimize(OptimizeReason::Interval);
                break;

            default:
                CDialog::OnTimer(id);
                break;
            }
        }

        void CMainDlg::CheckLowMemory()
        {
            if (settings_.lowMemoryPercent == 0 || running_)
            {
                return;
            }

            engine::MemoryStatus status;
            if (!engine::QueryMemoryStatus(status))
            {
                return;
            }

            const std::uint32_t freePercent = engine::Percent(status.availablePhysical, status.totalPhysical);
            if (freePercent >= settings_.lowMemoryPercent)
            {
                return;
            }

            const ULONGLONG now = ::GetTickCount64();
            if (lowMemoryRunHappened_ && now - lastLowMemoryRunTick_ < kLowMemoryMinGapMs)
            {
                return;
            }

            if (RequestOptimize(OptimizeReason::LowMemory))
            {
                lowMemoryRunHappened_ = true;
                lastLowMemoryRunTick_ = now;
            }
        }

        LRESULT CMainDlg::OnPowerBroadcastMsg(WPARAM wParam, LPARAM /*lParam*/)
        {
            if (wParam == PBT_APMRESUMEAUTOMATIC)
            {
                RestartTimers();
            }

            return TRUE;
        }

        // ---- Optimization -------------------------------------------------------------

        bool CMainDlg::RequestOptimize(OptimizeReason reason)
        {
            if (running_)
            {
                return false;
            }

            const std::uint32_t areas = engine::NormalizeAreaMask(settings_.areas);
            if (areas == 0)
            {
                return false;
            }

            JoinWorker();

            engine::RunOptions options;
            options.areas = areas;
            options.exclusions = settings_.exclusions;
            options.lowPriority = settings_.lowPriority;
            const HWND hwnd = m_hWnd;
            options.progress = [hwnd](std::size_t index, std::size_t count, engine::Area area)
            {
                ::PostMessageW(hwnd, WM_APP_PROGRESS, static_cast<WPARAM>(index),
                    MAKELPARAM(static_cast<WORD>(count), static_cast<WORD>(engine::AreaIndex(area))));
            };

            running_ = true;
            currentReason_ = reason;
            memoryPage_->SetBusy(true);
            UpdateTray();

            try
            {
                // C++11 has no init-capture: the lambda keeps its own copy of the options.
                worker_ = std::thread([this, hwnd, options]()
                {
                    engine::RunResult result = engine::RunOptimization(options);
                    {
                        const std::lock_guard<std::mutex> lock(resultMutex_);
                        pendingResult_ = std::move(result);
                    }
                    ::PostMessageW(hwnd, WM_APP_DONE, 0, 0);
                });
            }
            catch (const std::system_error&)
            {
                running_ = false;
                memoryPage_->SetBusy(false);
                UpdateTray();
                return false;
            }

            return true;
        }

        void CMainDlg::JoinWorker()
        {
            if (worker_.joinable())
            {
                worker_.join();
            }
        }

        LRESULT CMainDlg::OnRunProgress(WPARAM wParam, LPARAM lParam)
        {
            const auto index = static_cast<unsigned>(wParam);
            const unsigned count = LOWORD(lParam);
            const unsigned areaIndex = HIWORD(lParam);
            if (areaIndex < engine::kAreaOrder.size())
            {
                CString text;
                text.Format(LoadStr(IDS_OPTIMIZING_FMT), AreaDisplayName(engine::kAreaOrder[areaIndex]).GetString(),
                    index + 1, count);
                memoryPage_->SetStatusText(text, false);
            }
            return 0;
        }

        LRESULT CMainDlg::OnRunDone(WPARAM /*wParam*/, LPARAM /*lParam*/)
        {
            JoinWorker();
            {
                const std::lock_guard<std::mutex> lock(resultMutex_);
                lastResult_ = std::move(pendingResult_);
                pendingResult_ = engine::RunResult{};
            }

            running_ = false;

            const CString freed = FormatBytesUi(lastResult_.freedBytes);
            CString text;
            text.Format(LoadStr(IDS_FREED_FMT), freed.GetString(),
                static_cast<double>(lastResult_.durationMs) / 1000.0);
            memoryPage_->SetBusy(false);
            memoryPage_->SetStatusText(text, !lastResult_.AllSucceeded());
            RefreshMemory();

            if (settings_.showNotifications)
            {
                CString message;
                message.Format(LoadStr(IDS_NOTIFY_FMT), freed.GetString(), ReasonText(currentReason_).GetString());
                tray_.ShowNotification(LoadStr(IDS_APP_TITLE), message);
            }

            return 0;
        }

        void CMainDlg::ShowDetails()
        {
            MessageBox(FormatDetails(lastResult_), LoadStr(IDS_DETAILS_TITLE), MB_OK | MB_ICONINFORMATION);
        }

        LRESULT CMainDlg::OnHotKeyMsg(WPARAM wParam, LPARAM /*lParam*/)
        {
            if (static_cast<int>(wParam) == kHotkeyId)
            {
                RequestOptimize(OptimizeReason::Hotkey);
            }

            return 0;
        }

        // ---- Tray -----------------------------------------------------------------------

        LRESULT CMainDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam)
        {
            switch (LOWORD(lParam))
            {
            case NIN_SELECT:
            case NIN_KEYSELECT:
                ToggleWindow();
                break;

            case WM_MBUTTONUP:
                if (settings_.optimizeOnMiddleClick)
                {
                    RequestOptimize(OptimizeReason::Tray);
                }
                break;

            case WM_CONTEXTMENU:
                ShowTrayMenu(GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam));
                break;

            default:
                break;
            }

            return 0;
        }

        void CMainDlg::ShowTrayMenu(int x, int y)
        {
            CMenu menu;
            if (!menu.LoadMenu(IDR_TRAYMENU))
            {
                return;
            }

            CMenu* popup = menu.GetSubMenu(0);
            if (popup == nullptr)
            {
                return;
            }

            popup->EnableMenuItem(ID_TRAY_OPTIMIZE, MF_BYCOMMAND | (running_ ? MF_GRAYED : MF_ENABLED));
            popup->SetDefaultItem(ID_TRAY_SHOW);
            UINT flags = TPM_RIGHTBUTTON;
            flags |= ::GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0 ? TPM_RIGHTALIGN : TPM_LEFTALIGN;
            SetForegroundWindow();
            popup->TrackPopupMenu(flags, x, y, this);
            PostMessage(WM_NULL);
        }

        LRESULT CMainDlg::OnTaskbarCreated(WPARAM /*wParam*/, LPARAM /*lParam*/)
        {
            tray_.Add();
            UpdateTray();
            return 0;
        }

        void CMainDlg::OnTrayOptimize()
        {
            RequestOptimize(OptimizeReason::Tray);
        }

        void CMainDlg::OnTrayShow()
        {
            ShowMainWindow();
        }

        void CMainDlg::OnTrayExit()
        {
            DestroyWindow();
        }

        LRESULT CMainDlg::OnActivateRequest(WPARAM /*wParam*/, LPARAM /*lParam*/)
        {
            ShowMainWindow();
            return 0;
        }

        // ---- Window behaviour -----------------------------------------------------------

        void CMainDlg::ShowMainWindow()
        {
            ShowWindow(SW_SHOW);
            if (IsIconic())
            {
                ShowWindow(SW_RESTORE);
            }

            SetForegroundWindow();
            RefreshMemory();
        }

        void CMainDlg::HideToTray()
        {
            ShowWindow(SW_HIDE);
        }

        void CMainDlg::ToggleWindow()
        {
            if (IsWindowVisible() && !IsIconic())
            {
                HideToTray();
            }
            else
            {
                ShowMainWindow();
            }
        }

        void CMainDlg::OnSysCommand(UINT id, LPARAM lParam)
        {
            if ((id & 0xFFF0) == SC_MINIMIZE)
            {
                HideToTray();
                return;
            }

            CDialog::OnSysCommand(id, lParam);
        }

        void CMainDlg::OnClose()
        {
            if (settings_.closeToTray)
            {
                HideToTray();
            }
            else
            {
                DestroyWindow();
            }
        }

        void CMainDlg::OnDestroy()
        {
            KillTimer(kTimerRefresh);
            KillTimer(kTimerLowMemory);
            KillTimer(kTimerInterval);
            if (hotkeyRegistered_)
            {
                ::UnregisterHotKey(m_hWnd, kHotkeyId);
                hotkeyRegistered_ = false;
            }

            tray_.Remove();
            JoinWorker(); // an in-progress run finishes before the process exits
            CDialog::OnDestroy();
        }

        void CMainDlg::PostNcDestroy()
        {
            CDialog::PostNcDestroy();
            delete this;
        }

    } // namespace ui
} // namespace mp
