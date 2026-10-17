#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(StupidPlayerObject, PlayerObject) {
    struct Fields {
        CCSprite* p_soulSpr = nullptr;
    };

    bool init(int player, int ship, GJBaseGameLayer* gameLayer, CCLayer* layer, bool playLayer) {
        if (!PlayerObject::init(player, ship, gameLayer, layer, playLayer)) return false;
        
        if (!Mod::get()->getSettingValue<bool>("enable-soul")) return true;
        auto fields = m_fields.self();

        fields->p_soulSpr = CCSprite::createWithSpriteFrameName("soulBase.png"_spr);
        fields->p_soulSpr->setScale(1.5f);
        fields->p_soulSpr->setID("soul-sprite"_spr);
        this->addChild(fields->p_soulSpr);

        m_mainLayer->setVisible(false);
        m_robotSprite->setVisible(false);
        m_spiderSprite->setVisible(false);

        return true;
    }

    ccColor3B getColorToUse() {
        ccColor3B ret;
        std::string colorMode = Mod::get()->getSettingValue<std::string>("soulmode-colormode");
        auto gm = GameManager::sharedState();

        if (Mod::get()->getSettingValue<bool>("soulmode-static-color")) {
            ret = gm->colorForIdx(gm->getPlayerColor());
            return ret;
        }

        if (colorMode == "GD Color") {
            if (m_isShip) {
                ret = cocos2d::ccColor3B({ 255, 0, 255 });
            } else if (m_isBall) {
                ret = cocos2d::ccColor3B({255, 0, 0});
            } else if (m_isBird) {
                ret = cocos2d::ccColor3B({ 255, 145, 0 });
            } else if (m_isDart) {
                ret = cocos2d::ccColor3B({ 0, 255, 255 });
            } else if (m_isRobot) {
                ret = cocos2d::ccColor3B({255, 255, 255});
            } else if (m_isSpider) {
                ret = cocos2d::ccColor3B({ 140, 0, 255 });
            } else if (m_isSwing) {
                ret = cocos2d::ccColor3B({ 255, 255, 0 });
            } else {
                ret = cocos2d::ccColor3B({0, 255, 0});
            }
        } else if (colorMode == "UTDR Color") {
            if (m_isShip) {
                ret = cocos2d::ccColor3B({255, 0, 0});
            } else if (m_isBall) {
                ret = cocos2d::ccColor3B({0, 0, 255});
            } else if (m_isBird) {
                ret = cocos2d::ccColor3B({255, 0, 0});
            } else if (m_isDart) {
                ret = cocos2d::ccColor3B({215, 0, 215});
            } else if (m_isRobot) {
                ret = cocos2d::ccColor3B({255, 0, 0});
            } else if (m_isSpider) {
                ret = cocos2d::ccColor3B({0, 0, 255 });
            } else if (m_isSwing) {
                ret = cocos2d::ccColor3B({ 255, 255, 0 });
            } else {
                ret = cocos2d::ccColor3B({255, 0, 0});
            }
        }

        return ret;
    }

    void update(float p0) {
        PlayerObject::update(p0);

        if (!Mod::get()->getSettingValue<bool>("enable-soul")) return;
        auto fields = m_fields.self();
        auto mod = Mod::get();
        if (!fields->p_soulSpr) return;

        float targetScale;

        if (m_isDart) {
            targetScale = 1.0f;
        } else if (m_isSpider) {
            targetScale = 1.5f;
        } else {
            targetScale = 1.6f;
        }

        this->setRotation(0);
        fields->p_soulSpr->setScale(targetScale);
        fields->p_soulSpr->setColor(getColorToUse());
        
        if (!mod->getSettingValue<bool>("soulmode-keepat0rot")) {
            if (m_isSwing) {
                fields->p_soulSpr->setRotation(-90);
            } else {
                fields->p_soulSpr->setRotation(0);
                fields->p_soulSpr->setFlipY(m_isUpsideDown);
            }
        }

        if (m_ghostTrail) {
            m_ghostTrail->m_iconSprite = fields->p_soulSpr;
            m_ghostTrail->m_color = getColorToUse();

            if (!mod->getSettingValue<bool>("soulmode-keepat0rot")) {
                if (m_isSwing) {
                    m_ghostTrail->m_iconSprite->setRotation(-90);
                } else {
                    m_ghostTrail->m_iconSprite->setRotation(0);
                    m_ghostTrail->m_iconSprite->setFlipY(m_isUpsideDown);
                }
            } else {
                m_ghostTrail->m_iconSprite->setRotation(0);
                m_ghostTrail->m_iconSprite->setFlipY(false);
            }
        } 
    }
};