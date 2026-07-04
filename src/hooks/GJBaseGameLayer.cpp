#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../utils/RPUtils.cpp"

using namespace geode::prelude;

class $modify(GJBaseGameLayer) {
    struct Fields {
        int currentTextureIndex = 1;
        float timeSinceLastTextureUpdate = 0.f;
    };

    void update(float dt) {
        GJBaseGameLayer::update(dt);

        bool inEditor = m_isEditor;
        bool enableInEditor = Mod::get()->getSettingValue<bool>("enable-in-editor");

        if (inEditor && !enableInEditor) return;

        auto playLayer = typeinfo_cast<PlayLayer*>(this);

        if (playLayer && Mod::get()->getSettingValue<bool>("easyMode") && playLayer->m_isPracticeMode) {
            playLayer->onQuit();
        }

        m_fields->timeSinceLastTextureUpdate += dt;

        if (m_fields->timeSinceLastTextureUpdate >= 0.05f) {
            m_fields->timeSinceLastTextureUpdate = 0.f;
            m_fields->currentTextureIndex = m_fields->currentTextureIndex % 15 + 1;
        }

        if (!m_objects) return;

        auto frames = CCSpriteFrameCache::sharedSpriteFrameCache();
        auto texIndex = m_fields->currentTextureIndex;
        auto disableBlending = Mod::get()->getSettingValue<bool>("disableBlending");

        for (unsigned i = 0; i < m_objects->count(); ++i) {
            auto obj = typeinfo_cast<GameObject*>(m_objects->objectAtIndex(i));
            if (!obj) continue;

            if (obj->getZOrder() == -89 || obj->getZOrder() == -90) { // Yeah, the only way to get the back object for the portal is only with Z order, sadly, yes
                RPUtils::updateBackSprite(obj, frames, texIndex);
            }

            if (RPUtils::isPortalObject(obj->m_objectID)) {
                RPUtils::updateFrontSprite(obj, frames, texIndex);
                RPUtils::updatePortalParticle(obj, disableBlending);
            }
        }
    }
};
