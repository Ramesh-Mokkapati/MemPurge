// MemPurge - core/StartupTask.cpp
#define SECURITY_WIN32
#include <windows.h>
#include <security.h>
#include <taskschd.h>
#include <wrl/client.h>

#include "core/StartupTask.h"

#include "core/Bstr.h"
#include "core/Variant.h"

#include <vector>

using Microsoft::WRL::ComPtr;

namespace mp
{
    namespace core
    {
        namespace
        {
            HRESULT ConnectRootFolder(ComPtr<ITaskService>& service, ComPtr<ITaskFolder>& folder)
            {
                HRESULT hr = ::CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&service));
                if (FAILED(hr))
                {
                    return hr;
                }

                const Variant empty;
                hr = service->Connect(empty.get(), empty.get(), empty.get(), empty.get());
                if (FAILED(hr))
                {
                    return hr;
                }

                const Bstr root(L"\\");
                return service->GetFolder(root.get(), &folder);
            }

            std::wstring CurrentUserSamName()
            {
                ULONG size = 0;
                ::GetUserNameExW(NameSamCompatible, nullptr, &size);
                if (size == 0)
                {
                    return {};
                }

                std::vector<wchar_t> buffer(size + 1, L'\0');
                if (!::GetUserNameExW(NameSamCompatible, buffer.data(), &size))
                {
                    return {};
                }

                return std::wstring(buffer.data());
            }

        } // namespace

        bool IsStartupTaskRegistered()
        {
            ComPtr<ITaskService> service;
            ComPtr<ITaskFolder> folder;
            if (FAILED(ConnectRootFolder(service, folder)))
            {
                return false;
            }

            const Bstr name(kStartupTaskName);
            ComPtr<IRegisteredTask> task;
            return SUCCEEDED(folder->GetTask(name.get(), &task)) && task;
        }

        long RegisterStartupTask(const std::wstring& exePath)
        {
            ComPtr<ITaskService> service;
            ComPtr<ITaskFolder> folder;
            HRESULT hr = ConnectRootFolder(service, folder);
            if (FAILED(hr))
            {
                return hr;
            }

            ComPtr<ITaskDefinition> definition;
            if (FAILED(hr = service->NewTask(0, &definition)))
            {
                return hr;
            }

            // Principal: interactive token of the registering user, elevated.
            ComPtr<IPrincipal> principal;
            if (FAILED(hr = definition->get_Principal(&principal)))
            {
                return hr;
            }

            if (FAILED(hr = principal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN)) ||
                FAILED(hr = principal->put_RunLevel(TASK_RUNLEVEL_HIGHEST)))
            {
                return hr;
            }

            // Settings: run on battery, no execution time limit (tray app runs forever).
            ComPtr<ITaskSettings> settings;
            if (FAILED(hr = definition->get_Settings(&settings)))
            {
                return hr;
            }

            const Bstr noLimit(L"PT0S");
            if (FAILED(hr = settings->put_DisallowStartIfOnBatteries(VARIANT_FALSE)) ||
                FAILED(hr = settings->put_StopIfGoingOnBatteries(VARIANT_FALSE)) ||
                FAILED(hr = settings->put_ExecutionTimeLimit(noLimit.get())) ||
                FAILED(hr = settings->put_MultipleInstances(TASK_INSTANCES_IGNORE_NEW)))
            {
                return hr;
            }

            // Trigger: logon of the current user.
            ComPtr<ITriggerCollection> triggers;
            if (FAILED(hr = definition->get_Triggers(&triggers)))
            {
                return hr;
            }

            ComPtr<ITrigger> trigger;
            if (FAILED(hr = triggers->Create(TASK_TRIGGER_LOGON, &trigger)))
            {
                return hr;
            }

            ComPtr<ILogonTrigger> logonTrigger;
            if (FAILED(hr = trigger.As(&logonTrigger)))
            {
                return hr;
            }

            const std::wstring user = CurrentUserSamName();
            if (!user.empty())
            {
                const Bstr userId(user.c_str());
                if (FAILED(hr = logonTrigger->put_UserId(userId.get())))
                {
                    return hr;
                }
            }

            // Action: "<exe>" /Minimized
            ComPtr<IActionCollection> actions;
            if (FAILED(hr = definition->get_Actions(&actions)))
            {
                return hr;
            }

            ComPtr<IAction> action;
            if (FAILED(hr = actions->Create(TASK_ACTION_EXEC, &action)))
            {
                return hr;
            }

            ComPtr<IExecAction> execAction;
            if (FAILED(hr = action.As(&execAction)))
            {
                return hr;
            }

            const Bstr path(exePath.c_str());
            const Bstr arguments(L"/Minimized");
            if (!path.valid(exePath.c_str()) || !arguments.valid(L"/Minimized"))
            {
                return E_OUTOFMEMORY;
            }

            if (FAILED(hr = execAction->put_Path(path.get())) ||
                FAILED(hr = execAction->put_Arguments(arguments.get())))
            {
                return hr;
            }

            const Bstr name(kStartupTaskName);
            const Variant empty;
            const Variant emptySddl;
            ComPtr<IRegisteredTask> registered;
            hr = folder->RegisterTaskDefinition(name.get(), definition.Get(), TASK_CREATE_OR_UPDATE,
                empty.get(), empty.get(), TASK_LOGON_INTERACTIVE_TOKEN,
                emptySddl.get(), &registered);
            return hr;
        }

        long DeleteStartupTask()
        {
            ComPtr<ITaskService> service;
            ComPtr<ITaskFolder> folder;
            HRESULT hr = ConnectRootFolder(service, folder);
            if (FAILED(hr))
            {
                return hr;
            }

            const Bstr name(kStartupTaskName);
            hr = folder->DeleteTask(name.get(), 0);
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                return S_OK;
            }

            return hr;
        }

    } // namespace core
} // namespace mp
