// MemPurge - engine/Areas.cpp
#include "engine/Areas.h"

namespace mp
{
    namespace engine
    {

        std::uint32_t NormalizeAreaMask(std::uint32_t mask) noexcept
        {
            mask &= kAllAreasMask;
            if (HasArea(mask, Area::StandbyList))
            {
                mask &= ~ToMask(Area::StandbyListLowPriority);
            }

            return mask;
        }

        std::vector<Area> AreasFromMask(std::uint32_t mask)
        {
            std::vector<Area> areas;
            for (Area area : kAreaOrder)
            {
                if (HasArea(mask, area))
                {
                    areas.push_back(area);
                }
            }

            return areas;
        }

        std::uint32_t MaskFromAreas(const std::vector<Area>& areas) noexcept
        {
            std::uint32_t mask = 0;
            for (Area area : areas)
            {
                mask |= ToMask(area);
            }

            return mask;
        }

        std::size_t AreaIndex(Area area) noexcept
        {
            for (std::size_t i = 0; i < kAreaOrder.size(); ++i)
            {
                if (kAreaOrder[i] == area)
                {
                    return i;
                }
            }

            return 0;
        }

        const wchar_t* AreaSwitchName(Area area) noexcept
        {
            switch (area)
            {
            case Area::WorkingSet:
                return L"WorkingSet";

            case Area::SystemFileCache:
                return L"SystemFileCache";

            case Area::ModifiedPageList:
                return L"ModifiedPageList";

            case Area::StandbyList:
                return L"StandbyList";

            case Area::StandbyListLowPriority:
                return L"StandbyListLowPriority";

            case Area::CombinedPageList:
                return L"CombinedPageList";

            case Area::RegistryCache:
                return L"RegistryCache";

            case Area::ModifiedFileCache:
                return L"ModifiedFileCache";
            }

            return L"";
        }

    } // namespace engine
} // namespace mp
