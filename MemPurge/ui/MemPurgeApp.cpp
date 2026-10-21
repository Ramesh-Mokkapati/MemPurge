// MemPurge - ui/MemPurgeApp.cpp
// Application entry: command-line (headless) mode or the tray GUI.
#include "framework.h"

#include "MemPurgeApp.h"

#include <string>
#include <vector>

#include "MainDlg.h"
#include "ParentConsole.h"
#include "UiText.h"
#include "core/CommandLine.h"
#include "core/SettingsStore.h"
#include "engine/Optimizer.h"

namespace mp
{
    namespace ui
    {
        namespace
        {
            std::vector<std::wstring> CommandLineArgs()
            {
                std::vector<std::wstring> args;
                int argc = 0;
                LPWSTR* argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
                if (argv == nullptr)
                {
                    return args;
                }

                for (int i = 1; i < argc; ++i)
                {
                    args.emplace_back(argv[i]);
                }

                ::LocalFree(argv);
                return args;
            }

        } // namespace

        CMemPurgeApp theApp;

        BOOL CMemPurgeApp::InitInstance()
        {
            CWinApp::InitInstance();

            const core::CommandLineOptions options = core::ParseCommandLine(CommandLineArgs());

            if (options.HasError())
            {
                CString error;
                error.Format(LoadStr(IDS_UNKNOWN_SWITCH_FMT), options.unknownSwitches.front().c_str());
                ShowUsage(error);
                exitCode_ = 2;
                return FALSE;
            }

            if (options.help)
            {
                ShowUsage(CString());
                exitCode_ = 0;
                return FALSE;
            }

            if (options.IsHeadless())
            {
                // No window, no tray icon, no single-instance check.
                exitCode_ = RunHeadless(options.areas);
                return FALSE;
            }

            // ---- GUI mode ----
            if (!instance_.Acquire())
            {
                SingleInstance::ActivateExisting();
                exitCode_ = 0;
                return FALSE;
            }

            comInitialized_ = SUCCEEDED(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));

            INITCOMMONCONTROLSEX icc{};
            icc.dwSize = sizeof(icc);
            icc.dwICC = ICC_WIN95_CLASSES | ICC_TAB_CLASSES | ICC_HOTKEY_CLASS | ICC_PROGRESS_CLASS |
                ICC_UPDOWN_CLASS | ICC_STANDARD_CLASSES;
            ::InitCommonControlsEx(&icc);

            const core::Settings settings = core::LoadSettings();
            const bool startHidden = options.minimized || settings.startMinimized;

            auto* dlg = new CMainDlg(startHidden, instance_);
            m_pMainWnd = dlg;
            if (!dlg->CreateModeless())
            {
                m_pMainWnd = nullptr;
                delete dlg;
                exitCode_ = 1;
                return FALSE;
            }

            guiMode_ = true;
            return TRUE;
        }

        int CMemPurgeApp::ExitInstance()
        {
            const int baseCode = CWinApp::ExitInstance();
            if (comInitialized_)
            {
                ::CoUninitialize();
                comInitialized_ = false;
            }

            return guiMode_ ? baseCode : exitCode_;
        }

        void CMemPurgeApp::ShowUsage(const CString& error)
        {
            CString text;
            if (!error.IsEmpty())
            {
                text = error + L"\n\n";
            }

            text += LoadStr(IDS_USAGE);

            ParentConsole console;
            if (console.attached())
            {
                console.WriteLine(text);
            }
            else
            {
                ::MessageBoxW(nullptr, text, LoadStr(IDS_APP_TITLE),
                    MB_OK | (error.IsEmpty() ? MB_ICONINFORMATION : MB_ICONWARNING));
            }
        }

        int CMemPurgeApp::RunHeadless(std::uint32_t areas)
        {
            engine::RunOptions options;
            options.areas = areas;
            options.exclusions = core::LoadSettings().exclusions; // exclusions apply in CLI mode too
            const engine::RunResult result = engine::RunOptimization(options);

            ParentConsole console;
            for (const auto& r : result.results)
            {
                console.WriteLine(FormatResultLine(r));
            }

            CString freed;
            freed.Format(LoadStr(IDS_CLI_FREED_FMT), FormatBytesUi(result.freedBytes).GetString());
            console.WriteLine(freed);

            return result.AllSucceeded() ? 0 : 1;
        }

    } // namespace ui
} // namespace mp
