// MemPurge - engine/ProcessName.cpp
#include <windows.h>

#include "engine/ProcessName.h"

#include <algorithm>
#include <cwctype>

namespace mp
{
    namespace engine
    {
        namespace
        {
            std::wstring Trim(const std::wstring& s)
            {
                std::size_t first = 0;
                std::size_t last = s.size();
                while (first < last && std::iswspace(s[first]))
                {
                    ++first;
                }

                while (last > first && std::iswspace(s[last - 1]))
                {
                    --last;
                }

                return s.substr(first, last - first);
            }

            std::wstring ToLowerInvariant(const std::wstring& s)
            {
                std::wstring result(s);
                if (result.empty())
                {
                    return result;
                }

                const int len = static_cast<int>(result.size());
                std::wstring lowered(result.size(), L'\0');
                const int written = ::LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_LOWERCASE, result.c_str(), len,
                    &lowered[0], len, nullptr, nullptr, 0);
                if (written == len)
                {
                    return lowered;
                }

                // Fallback: per-character C runtime lower-casing.
                std::transform(result.begin(), result.end(), result.begin(),
                    [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
                return result;
            }

        } // namespace

        std::wstring NormalizeProcessName(const std::wstring& name)
        {
            std::wstring s = Trim(name);
            const std::size_t sep = s.find_last_of(L"\\/");
            if (sep != std::wstring::npos)
            {
                s = s.substr(sep + 1);
            }

            std::wstring lowered = ToLowerInvariant(s);
            const std::wstring kExe = L".exe";
            if (lowered.size() >= kExe.size() &&
                lowered.compare(lowered.size() - kExe.size(), kExe.size(), kExe) == 0)
            {
                lowered.resize(lowered.size() - kExe.size());
            }

            return Trim(lowered);
        }

        std::vector<std::wstring> NormalizeNameList(const std::vector<std::wstring>& names)
        {
            std::vector<std::wstring> result;
            result.reserve(names.size());
            for (const auto& name : names)
            {
                std::wstring n = NormalizeProcessName(name);
                if (!n.empty())
                {
                    result.push_back(std::move(n));
                }
            }

            std::sort(result.begin(), result.end());
            result.erase(std::unique(result.begin(), result.end()), result.end());
            return result;
        }

    } // namespace engine
} // namespace mp
