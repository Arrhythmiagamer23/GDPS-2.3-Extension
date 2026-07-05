#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include "../utils/Utils.hpp"
#include "../utils/IconUtils.hpp"

using namespace geode::prelude;

enum class CustomIcons
{
    Cube = 383,
    Ship = 70,
    Ball = 82,
    UFO = 62,
    Wave = 51,
    Swing = 5,
    Jetpack = 1
};

class $modify(OdysseyGameManager, GameManager)
{
    void firstLoad()
    {
        GameManager::firstLoad();
    }

    int countForType(IconType icon)
    {
        switch (icon)
        {
        case IconType::Cube:
            return 485 + (int)CustomIcons::Cube;
        case IconType::Ship:
            return 169 + (int)CustomIcons::Ship;
        case IconType::Ball:
            return 118 + (int)CustomIcons::Ball;
        case IconType::Ufo:
            return 149 + (int)CustomIcons::UFO;
        case IconType::Wave:
            return 96 + (int)CustomIcons::Wave;
        case IconType::Robot:
            return 68;
        case IconType::Spider:
            return 69;
        case IconType::Swing:
            return 43 + (int)CustomIcons::Swing;
         case IconType::Jetpack:
            return 8 + (int)CustomIcons::Jetpack;
        case IconType::Special:
            return 7;
        case IconType::DeathEffect:
            return 20;
        case IconType::ShipFire:
            return 6;
        case IconType::Item:
            return 21;
        default:
            //  Yes, this solution is terrible, but I don't know how to patch bytes.
            return GameManager::countForType(icon);
        }
    }

    bool isIconUnlocked(int id, IconType type)
    {
        if (IconUtils::isIconCustom(id, type))
        {
            if (IconUtils::isCustomVehicle(type))
            {
                auto obj = get()->m_valueKeeper->valueForKey(IconUtils::getItemKey(id, IconUtils::iconTypeToInt(type)));
                return (id == 1) ? true : obj->intValue() == 1;
            }
            else
            {
                return GameManager::isIconUnlocked(id, type);
            }
        }

        if (type == IconType::Item)
            return GameManager::isIconUnlocked(id, type);

        return true;
    }

    bool isColorUnlocked(int id, UnlockType type)
    {
        return true;
    }

    void dataLoaded(DS_Dictionary *dict)
    {
        GameManager::dataLoaded(dict);
    }

    gd::string sheetNameForIcon(int iconID, int iconType)
    {
        if (iconType >= 900)
        {
            const char *gamemode[4] = {"boat", "drone", "slider", "minecart"};

            return fmt::format("icons/{}_{:02}", gamemode[iconType - 901], iconID);
        }

        auto ret = GameManager::sheetNameForIcon(iconID, iconType);
        return ret;
    };

    CCTexture2D* loadIcon(int id, int type, int requestID) {
        if (type>=900) type=1;

        CCTexture2D* ret = GameManager::loadIcon(id, type, requestID);
        return ret;
    }
};
