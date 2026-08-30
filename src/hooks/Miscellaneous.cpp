#include <Geode/modify/MenuGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>

#include <undefined0.icon_ninja/include/events.hpp>

#include "../internal/CosmicClone.hpp"
#include "../internal/ShaderManager.hpp"

using namespace geode::prelude;

// Pause Layer Button

class $modify(ClonesPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto spr = CCSprite::createWithSpriteFrameName("cube_1.png"_spr);
        spr->setRotation(55);
        spr->setShaderProgram(ShaderManager::get().getCosmicShader());

        auto btnSpr = CircleButtonSprite::create(spr, CircleBaseColor::Green, CircleBaseSize::MediumAlt);
        btnSpr->setScale(.6f);
        auto btn = CCMenuItemExt::createSpriteExtra(btnSpr, [](auto) {
           openSettingsPopup(Mod::get(), true);
        });
        spr->setScale(.75);
        btn->setID("clones-settings-btn"_spr);

        auto menu = getChildByID("left-button-menu");
        menu->addChild(btn);
        menu->updateLayout();
    }
};

// Easter Egg

class $modify(ClonesMenuGameLayer, MenuGameLayer) {
    struct Fields {
        bool m_wasCloneLast = false;
    };

    void resetPlayer() {
        MenuGameLayer::resetPlayer();

        if (random::generate(0, 1000) == 309) {
            CosmicClone::updateShaderForPlayer(m_playerObject, ShaderManager::get().getCosmicShader(), true);
            m_fields->m_wasCloneLast = true;
        } else if (m_fields->m_wasCloneLast) {
            CosmicClone::updateShaderForPlayer(m_playerObject, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor), true);
            m_fields->m_wasCloneLast = false;
        }
    }
};


$on_mod(Loaded) {
    ninja::NewPlayerEvent().listen([](PlayerObject* player){
        if (random::generate(0, 1000) == 309) {
            CosmicClone::updateShaderForPlayer(player, ShaderManager::get().getCosmicShader(), true);
        }
    }).leak();
}
