// MemPurge - engine/MemoryStatus.cpp
#include <windows.h>

#include "engine/MemoryStatus.h"

namespace mp
{
    namespace engine
    {
        std::uint32_t Percent(std::uint64_t part, std::uint64_t whole) noexcept
        {
            if (whole == 0)
            {
                return 0;
            }

            if (part >= whole)
            {
                return 100;
            }

            // part < whole, so part * 100 cannot overflow for any realistic memory size;
            // guard anyway by scaling down very large values.
            while (whole > (UINT64_MAX / 200))
            {
                part >>= 1;
                whole >>= 1;
            }

            const std::uint64_t pct = (part * 100 + whole / 2) / whole;
            return static_cast<std::uint32_t>(pct > 100 ? 100 : pct);
        }

        MemoryStatus ComputeMemoryStatus(std::uint64_t totalPhys, std::uint64_t availPhys,
            std::uint64_t totalPageFile, std::uint64_t availPageFile) noexcept
        {
            MemoryStatus s;
            s.totalPhysical = totalPhys;
            s.availablePhysical = availPhys > totalPhys ? totalPhys : availPhys;
            s.usedPhysical = s.totalPhysical - s.availablePhysical;
            s.usedPercent = Percent(s.usedPhysical, s.totalPhysical);
            s.commitLimit = totalPageFile;
            s.commitTotal = availPageFile > totalPageFile ? 0 : totalPageFile - availPageFile;
            s.commitPercent = Percent(s.commitTotal, s.commitLimit);
            return s;
        }

        std::uint64_t ComputeFreedBytes(std::uint64_t availBefore, std::uint64_t availAfter) noexcept
        {
            return availAfter > availBefore ? availAfter - availBefore : 0;
        }

        bool QueryMemoryStatus(MemoryStatus& status) noexcept
        {
            MEMORYSTATUSEX ms{};
            ms.dwLength = sizeof(ms);
            if (!::GlobalMemoryStatusEx(&ms))
            {
                return false;
            }

            status = ComputeMemoryStatus(ms.ullTotalPhys, ms.ullAvailPhys, ms.ullTotalPageFile,
                ms.ullAvailPageFile);
            return true;
        }

    } // namespace engine
} // namespace mp
