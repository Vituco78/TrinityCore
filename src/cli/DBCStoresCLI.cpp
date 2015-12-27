#include "DBCStoresCLI.h"
#include <msclr\marshal_cppstd.h>

using namespace msclr::interop;

namespace DBCStoresCLI
{
    void DBCStores::Initialize(String^ dbcsPath)
    {
        dbcsFolderPath = dbcsPath;
        if (!dbcsFolderPath->EndsWith("\\"))
            dbcsFolderPath += "\\";
        if (dbcsFolderPath->EndsWith("dbc\\"))
            dbcsFolderPath = dbcsFolderPath->Substring(0, dbcsFolderPath->Length - 4);
        loaded = false;
    }

    void DBCStores::LoadDBCs()
    {
        if (!loaded)
        {
            loaded = true;
            LoadDBCStores(marshal_as<std::string>(dbcsFolderPath->ToString()));
        }
    }

    List<Point^>^ DBCStores::GetAchievementExploreLocations(float x, float y, float z, int mapID)
    {
        auto locations = gcnew List<Point^>();
        uint32 worldMapAreaId = 0;

        for (uint32 index = 0; index < sWorldMapAreaStore.GetNumRows(); index++)
        {
            auto worldMapArea = sWorldMapAreaStore.LookupEntry(index);
            if (!worldMapArea)
                continue;

            if (worldMapArea->map_id != mapID)
                continue;

            if (worldMapArea->x2 <= x && x <= worldMapArea->x1 && worldMapArea->y2 <= y && y <= worldMapArea->y1)
            {
                worldMapAreaId = worldMapArea->ID;
                break;
            }
        }

        if (worldMapAreaId == 0)
            return locations;

        uint32 worldMapOverlayId = 0;
        for (uint32 index = 0; index < sWorldMapOverlayStore.GetNumRows(); index++)
        {
            auto worldMapOverlay = sWorldMapOverlayStore.LookupEntry(index);
            if (!worldMapOverlay)
                continue;

            if (worldMapOverlay->worldMapAreaId == worldMapAreaId)
            {
                worldMapOverlayId = worldMapOverlay->ID;
                break;
            }
        }

        if (worldMapOverlayId == 0)
            return locations;

        uint32 achievementId = 0;
        for (uint32 index = 0; index < sAchievementCriteriaStore.GetNumRows(); index++)
        {
            auto achievementCriteria = sAchievementCriteriaStore.LookupEntry(index);
            if (!achievementCriteria)
                continue;

            if (achievementCriteria->requiredType != ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA)
                continue;

            if (achievementCriteria->explore_area.areaReference == worldMapOverlayId)
            {
                achievementId = achievementCriteria->referredAchievement;
                break;
            }
        }

        if (achievementId == 0)
            return locations;

        for (uint32 index = 0; index < sAchievementCriteriaStore.GetNumRows(); index++)
        {
            auto achievementCriteria = sAchievementCriteriaStore.LookupEntry(index);
            if (!achievementCriteria)
                continue;

            if (achievementCriteria->requiredType != ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA)
                continue;

            if (achievementCriteria->referredAchievement == achievementId)
            {
                auto worldMapOverlay = sWorldMapOverlayStore.LookupEntry(achievementCriteria->explore_area.areaReference);
                if (!worldMapOverlay)
                    continue;

                auto areaTableEntry = GetAreaEntryByAreaID(worldMapOverlay->areatableID[0]);
                if (!areaTableEntry)
                    continue;

                float x, y, z;
                Map::GetXYZFromAreaId(areaTableEntry->exploreFlag, mapID, x, y, z);
                locations->Add(gcnew Point(x, y, z));
            }
        }

        return locations;
    }
}
