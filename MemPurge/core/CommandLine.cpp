// MemPurge - core/CommandLine.cpp
#include <windows.h>

#include "core/CommandLine.h"

#include <cwchar>

#include "engine/Areas.h"

namespace mp
{
    namespace core
    {
        namespace
        {

            bool EqualsIgnoreCase(const std::wstring& a, const wchar_t* b)
            {
                return ::CompareStringOrdinal(a.c_str(), static_cast<int>(a.size()), b,
                    static_cast<int>(std::wcslen(b)), TRUE) == CSTR_EQUAL;
            }

        } // namespace

        CommandLineOptions ParseCommandLine(const std::vector<std::wstring>& args)
        {
            CommandLineOptions options;
            std::uint32_t mask = 0;

            for (const auto& arg : args)
            {
                if (arg.empty())
                {
                    continue;
                }

                if (arg.size() < 2 || (arg[0] != L'/' && arg[0] != L'-'))
                {
                    options.unknownSwitches.push_back(arg);
                    continue;
                }

                const std::wstring name = arg.substr(1);

                if (name == L"?" || EqualsIgnoreCase(name, L"Help"))
                {
                    options.help = true;
                    continue;
                }

                if (EqualsIgnoreCase(name, L"Minimized"))
                {
                    options.minimized = true;
                    continue;
                }

                bool matched = false;
                for (engine::Area area : engine::kAreaOrder)
                {
                    if (EqualsIgnoreCase(name, engine::AreaSwitchName(area)))
                    {
                        mask |= engine::ToMask(area);
                        matched = true;
                        break;
                    }
                }

                if (!matched)
                {
                    options.unknownSwitches.push_back(arg);
                }
            }

            options.areas = engine::NormalizeAreaMask(mask);
            return options;
        }

    } // namespace core
} // namespace mp
