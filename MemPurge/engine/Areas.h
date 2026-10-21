// MemPurge - engine/Areas.h
// Memory areas, their bitmask values and execution order (SPEC section 3).
// Pure helpers: no Windows calls.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace mp
{
    namespace engine
    {
        enum class Area : std::uint32_t
        {
            WorkingSet = 0x01,
            SystemFileCache = 0x02,
            ModifiedPageList = 0x04,
            StandbyList = 0x08,
            StandbyListLowPriority = 0x10,
            CombinedPageList = 0x20,
            RegistryCache = 0x40,
            ModifiedFileCache = 0x80,
        };

        constexpr std::size_t kAreaCount = 8;

        // Execution order of one optimize run (SPEC section 3 table order).
        constexpr std::array<Area, kAreaCount> kAreaOrder =
        { {
            Area::WorkingSet,       Area::SystemFileCache,        Area::ModifiedPageList,
            Area::StandbyList,      Area::StandbyListLowPriority, Area::CombinedPageList,
            Area::RegistryCache,    Area::ModifiedFileCache,
        } };

        constexpr std::uint32_t kAllAreasMask = 0xFFu;

        constexpr std::uint32_t ToMask(Area area) noexcept
        {
            return static_cast<std::uint32_t>(area);
        }

        constexpr bool HasArea(std::uint32_t mask, Area area) noexcept
        {
            return (mask & ToMask(area)) != 0;
        }

        // Drops unknown bits and enforces StandbyList / StandbyListLowPriority mutual
        // exclusion (full StandbyList wins when both are set).
        std::uint32_t NormalizeAreaMask(std::uint32_t mask) noexcept;

        // Areas contained in the mask, in execution order. Does not normalize.
        std::vector<Area> AreasFromMask(std::uint32_t mask);

        std::uint32_t MaskFromAreas(const std::vector<Area>& areas) noexcept;

        // Zero-based position of the area in kAreaOrder.
        std::size_t AreaIndex(Area area) noexcept;

        // Command-line switch name without prefix, e.g. L"WorkingSet".
        const wchar_t* AreaSwitchName(Area area) noexcept;

    } // namespace engine
} // namespace mp
