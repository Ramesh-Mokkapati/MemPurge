// MemPurge - ui/ParentConsole.cpp
#include "framework.h"

#include "ParentConsole.h"

#include <string>

namespace mp
{
    namespace ui
    {
        ParentConsole::ParentConsole()
            : attached_(::AttachConsole(ATTACH_PARENT_PROCESS) != FALSE)
            , utf8_(false)
            , out_(nullptr)
        {
            if (!attached_)
            {
                return;
            }

            out_ = ::GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD mode = 0;
            const bool redirected = out_ != nullptr && out_ != INVALID_HANDLE_VALUE &&
                ::GetFileType(out_) != FILE_TYPE_UNKNOWN &&
                !::GetConsoleMode(out_, &mode);
            if (redirected)
            {
                utf8_ = true; // pipe or file: write UTF-8 bytes
                return;
            }

            console_.reset(::CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0,
                nullptr));
            out_ = console_.get();
            // The shell prompt was already printed; start on a fresh line.
            WriteLine(CString());
        }

        ParentConsole::~ParentConsole()
        {
            if (attached_)
            {
                console_.reset();
                ::FreeConsole();
            }
        }

        bool ParentConsole::attached() const noexcept
        {
            return attached_;
        }

        void ParentConsole::WriteLine(const CString& text)
        {
            if (!attached_ || out_ == nullptr || out_ == INVALID_HANDLE_VALUE)
            {
                return;
            }

            CString line = text;
            line.Replace(L"\r\n", L"\n");
            line.Replace(L"\n", L"\r\n");
            line += L"\r\n";
            if (utf8_)
            {
                const int bytes = ::WideCharToMultiByte(CP_UTF8, 0, line.GetString(), line.GetLength(),
                    nullptr, 0, nullptr, nullptr);
                if (bytes <= 0)
                {
                    return;
                }

                std::string buffer(static_cast<std::size_t>(bytes), '\0');
                ::WideCharToMultiByte(CP_UTF8, 0, line.GetString(), line.GetLength(), &buffer[0], bytes,
                    nullptr, nullptr);
                DWORD written = 0;
                ::WriteFile(out_, buffer.data(), static_cast<DWORD>(buffer.size()), &written, nullptr);
            }
            else
            {
                DWORD written = 0;
                ::WriteConsoleW(out_, line.GetString(), static_cast<DWORD>(line.GetLength()), &written, nullptr);
            }
        }

    } // namespace ui
} // namespace mp
