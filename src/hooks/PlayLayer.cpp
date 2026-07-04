#include <Geode/modify/PlayLayer.hpp>
#include "../utils/RPUtils.cpp"

using namespace geode::prelude;

class $modify(PlayLayer) {
    void addObject(GameObject * obj) {
        PlayLayer::addObject(obj);

        if (Mod::get()->getSettingValue<bool>("freeMode")) {
            auto effect = typeinfo_cast<EffectGameObject*>(obj);

            if (effect) {
                effect->m_cameraIsFreeMode = true;
            }
        }
    }

    void togglePracticeMode(bool practiceMode) {
        if (!Mod::get()->getSettingValue<bool>("easyMode")) {
            PlayLayer::togglePracticeMode(practiceMode);
        }
    }

    void resetLevel() {
        static const std::vector<std::tuple<int, int, int>> portalTypes = {
            {31790, 6, 3},
        };

        static const std::unordered_map<std::string, int> nameToId = {
            {"Cube", 12},
            {"Ship", 13},
            {"Ball", 47},
            {"Ufo", 111},
            {"Wave", 660},
            {"Robot", 745},
            {"Spider", 1331},
            {"Swing", 1933}
        };

        auto easyMode = Mod::get()->getSettingValue<bool>("easyMode");
        auto disableBlending = Mod::get()->getSettingValue<bool>("disableBlending");
        bool skipAll = easyMode && (std::rand() % 100) < 50;

        if (m_objects && !skipAll) {
            std::unordered_set<int> excludedIDs;

            std::string wildcard1 = Mod::get()->getSettingValue<std::string>("wildcard1");
            std::string wildcard2 = Mod::get()->getSettingValue<std::string>("wildcard2");
            std::string wildcard3 = Mod::get()->getSettingValue<std::string>("wildcard3");
            std::string wildcard4 = Mod::get()->getSettingValue<std::string>("wildcard4");

            if (nameToId.count(wildcard1)) excludedIDs.insert(nameToId.at(wildcard1));
            if (nameToId.count(wildcard2)) excludedIDs.insert(nameToId.at(wildcard2));
            if (nameToId.count(wildcard3)) excludedIDs.insert(nameToId.at(wildcard3));
            if (nameToId.count(wildcard4)) excludedIDs.insert(nameToId.at(wildcard4));

            for (unsigned i = 0; i < m_objects->count(); ++i) {
                auto obj = typeinfo_cast<GameObject*>(m_objects->objectAtIndex(i));

                if (!obj || excludedIDs.count(obj->m_objectID)) continue;

                std::vector<std::tuple<int, int, int>> candidates;
                bool isPortal = false;

                for (auto& portal : portalTypes) {
                    int portalID = std::get<0>(portal);

                    if (portalID == obj->m_objectID) {
                        isPortal = true;
                    }
                    else if (!excludedIDs.count(portalID)) {
                        candidates.push_back(portal);
                    }
                }

                if (!isPortal || candidates.empty()) continue;

                int r = std::rand() % 100;
                int chosenIndex = -1;

                if (easyMode) {
                    float step = 50.f / candidates.size();
                    float threshold = 50.f;

                    if (r < threshold) continue;

                    for (unsigned j = 0; j < candidates.size(); ++j) {
                        threshold += step;

                        if (r < threshold) {
                            chosenIndex = j;
                            break;
                        }
                    }
                }
                else {
                    chosenIndex = std::rand() % candidates.size();
                }

                if (chosenIndex == -1) continue;

                auto newPortal = candidates[chosenIndex];
                obj->m_objectID = std::get<0>(newPortal);
                obj->m_objectType = static_cast<GameObjectType>(std::get<1>(newPortal));
                obj->m_savedObjectType = static_cast<GameObjectType>(std::get<1>(newPortal));

                RPUtils::updatePortalParticle(obj, disableBlending);
            }
        }

        PlayLayer::resetLevel();
    }
};