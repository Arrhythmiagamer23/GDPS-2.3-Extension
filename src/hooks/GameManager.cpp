#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include "../layers/OdysseySelectLayer.hpp"
#include "../utils/Utils.hpp"
#include "../utils/IconUtils.hpp"

using namespace geode::prelude;

enum class CustomIcons
{
    Cube = 35,
    Ship = 10,
    Ball = 12,
    UFO = 10,
    Wave = 6,
    Swing = 5,
    Jetpack = 1
};

class $modify(OdysseyGameManager, GameManager)
{
    void firstLoad()
    {
        GameManager::firstLoad();
        Mod::get()->setSavedValue<int>("Orbs", 0);
    };

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

    void returnToLastScene(GJGameLevel *level)
    {
        if (level->m_levelType == GJLevelType::Main)
        {
            auto page = Odyssey::getIslandForLevel(level->m_levelID);
            auto bgMusic = (page == 3) ? "SecretLoop02.mp3"_spr : fmt::format("IslandLoop{:02}.mp3"_spr, page + 1);

            CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, OdysseySelectLayer::scene(page)));
            GameManager::sharedState()->fadeInMusic(bgMusic);
            return;
        }

        GameManager::returnToLastScene(level);
    }

    void reportPercentageForLevel(int levelID, int percent, bool isPractice)
    {
        GameManager::reportPercentageForLevel(levelID, percent, isPractice);
        log::info("Level: {}, {}, {}", levelID, percent, isPractice);

        std::string achievementID = fmt::format("geometry.ach.level{:02}{}", levelID - 7000, isPractice ? "a" : "b");

        GameManager::sharedState()->reportAchievementWithID(achievementID.c_str(), percent, false);
    };

    void reportAchievementWithId(const char *ach, int perc, bool flag)
    {
        GameManager::reportAchievementWithID(ach, perc, flag);

        log::debug("ACH: {}, Percent: {}, Flag: {}", ach, perc, flag);
    };

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
