// MemPurge - engine/MemoryStatus.h
// Physical and commit memory status (SPEC section 3.3).
#pragma once

#include <cstdint>

namespace mp
{
    namespace engine
    {
        struct MemoryStatus
        {
            std::uint64_t totalPhysical = 0;
            std::uint64_t availablePhysical = 0;
            std::uint64_t usedPhysical = 0;
            std::uint32_t usedPercent = 0;      // 0..100
            std::uint64_t commitTotal = 0;      // ullTotalPageFile - ullAvailPageFile
            std::uint64_t commitLimit = 0;      // ullTotalPageFile
            std::uint32_t commitPercent = 0;    // 0..100
        };

        // Pure: builds a MemoryStatus from GlobalMemoryStatusEx fields.
        MemoryStatus ComputeMemoryStatus(std::uint64_t totalPhys, std::uint64_t availPhys,
            std::uint64_t totalPageFile,
            std::uint64_t availPageFile) noexcept;

        // Pure: rounded percentage part/whole, clamped to 0..100 (0 when whole is 0).
        std::uint32_t Percent(std::uint64_t part, std::uint64_t whole) noexcept;

        // Pure: freed bytes = after - before, clamped to >= 0.
        std::uint64_t ComputeFreedBytes(std::uint64_t availBefore,
            std::uint64_t availAfter) noexcept;

        // Reads the current status with GlobalMemoryStatusEx. Returns false on failure.
        bool QueryMemoryStatus(MemoryStatus& status) noexcept;

    } // namespace engine
} // namespace mp
