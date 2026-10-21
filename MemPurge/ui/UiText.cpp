// MemPurge - ui/UiText.cpp
#include "framework.h"

#include "UiText.h"

#include "core/SizeFormat.h"
#include "engine/OsVersion.h"

namespace mp
{
    namespace ui
    {
        CString LoadStr(UINT id)
        {
            CString s;
            VERIFY(s.LoadString(id));
            return s;
        }

        CString FormatBytesUi(std::uint64_t bytes)
        {
            static const core::ByteUnitNames units =
            { {
                std::wstring(LoadStr(IDS_UNIT_B).GetString()),  std::wstring(LoadStr(IDS_UNIT_KB).GetString()),
                std::wstring(LoadStr(IDS_UNIT_MB).GetString()), std::wstring(LoadStr(IDS_UNIT_GB).GetString()),
                std::wstring(LoadStr(IDS_UNIT_TB).GetString()),
            } };

            return CString(core::FormatBytes(bytes, units).c_str());
        }

        CString AreaDisplayName(engine::Area area)
        {
            return LoadStr(IDS_AREA_FIRST + static_cast<UINT>(engine::AreaIndex(area)));
        }

        CString StatusText(engine::AreaStatus status)
        {
            switch (status)
            {
            case engine::AreaStatus::Success:
                return LoadStr(IDS_STATUS_SUCCESS);

            case engine::AreaStatus::NotSupported:
                return LoadStr(IDS_STATUS_NOTSUPPORTED);

            case engine::AreaStatus::PrivilegeMissing:
                return LoadStr(IDS_STATUS_PRIVILEGE);

            case engine::AreaStatus::Failed:
                break;
            }

            return LoadStr(IDS_STATUS_FAILED);
        }

        CString ReasonText(OptimizeReason reason)
        {
            switch (reason)
            {
            case OptimizeReason::Manual:
                return LoadStr(IDS_REASON_MANUAL);

            case OptimizeReason::Hotkey:
                return LoadStr(IDS_REASON_HOTKEY);

            case OptimizeReason::Interval:
                return LoadStr(IDS_REASON_INTERVAL);

            case OptimizeReason::LowMemory:
                return LoadStr(IDS_REASON_LOWMEMORY);

            case OptimizeReason::Tray:
                break;
            }

            return LoadStr(IDS_REASON_TRAY);
        }

        CString FormatResultLine(const engine::AreaResult& result)
        {
            CString line;
            // NotSupported already carries its full meaning in the status text.
            if (result.status == engine::AreaStatus::Success ||
                result.status == engine::AreaStatus::NotSupported || result.win32Error == 0)
            {
                line.Format(LoadStr(IDS_RESULT_LINE_FMT), AreaDisplayName(result.area).GetString(),
                    StatusText(result.status).GetString());
            }
            else
            {
                line.Format(LoadStr(IDS_RESULT_LINE_ERR_FMT), AreaDisplayName(result.area).GetString(),
                    StatusText(result.status).GetString(), result.win32Error, result.message.c_str());
            }

            return line;
        }

        CString FormatDetails(const engine::RunResult& result)
        {
            const engine::OsVersion os = engine::GetOsVersion();
            CString text;
            text.Format(LoadStr(IDS_OS_FMT), os.major, os.minor, os.build);
            text += L"\n";
            for (const auto& r : result.results)
            {
                text += L"\n";
                text += FormatResultLine(r);
            }

            return text;
        }

    } // namespace ui
} // namespace mp
