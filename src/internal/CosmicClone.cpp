#include "CosmicClone.hpp"

#include <Geode/cocos/shaders/CCShaderCache.h>

#include "ShaderManager.hpp"
#include "../Utils.hpp"

using namespace geode::prelude;

int getFrame(const IconType type) {
    const auto gm = GameManager::sharedState();
    switch (type) {
        default: return gm->getPlayerFrame();
        case IconType::Ship: return gm->getPlayerShip();
        case IconType::Ball: return gm->getPlayerBall();
        case IconType::Ufo: return gm->getPlayerBird();
        case IconType::Wave: return gm->getPlayerDart();
        case IconType::Robot: return gm->getPlayerRobot();
        case IconType::Spider: return gm->getPlayerSpider();
        case IconType::Swing: return gm->getPlayerSwing();
        case IconType::Jetpack: return gm->getPlayerJetpack();
    }
}

CosmicPlayerObject* CosmicPlayerObject::createCosmic(int player, int ship, GJBaseGameLayer* gameLayer, CCLayer* layer,bool playLayer) {
    auto ret = static_cast<CosmicPlayerObject*>(PlayerObject::create(player, ship, gameLayer, layer, playLayer));
    ret->m_fields->m_isCosmic = true;
    return ret;
}

void CosmicClone::init(const int delay, bool plat, GJBaseGameLayer* gjbgl) {
    m_delay = delay;
    if (isAprilFools()) {
        m_sillyScale = random::generate(.5, 2);
    }

    auto gm = GameManager::sharedState();
    m_p1 = CosmicPlayerObject::createCosmic(gm->getPlayerFrame(), 1, gjbgl, gjbgl, PlayLayer::get());
    m_p1->m_fields->m_clone = this;
    m_p1->setID(fmt::format("cosmic-clone-{}"_spr, delay));
    m_p1->togglePlatformerMode(plat);

    auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
    m_p2 = CosmicPlayerObject::createCosmic(sdi ? sdi->getSavedValue<int>("cube", gm->getPlayerFrame()) : gm->getPlayerFrame(), 1, gjbgl, gjbgl, PlayLayer::get());
    m_p2->m_fields->m_clone = this;
    m_p2->setID(fmt::format("cosmic-clone-dual-{}"_spr, delay));
    m_p2->togglePlatformerMode(plat);
}

std::shared_ptr<CosmicClone> CosmicClone::create(const int delay, bool plat, GJBaseGameLayer* gjbgl) {
    auto ret = std::make_shared<CosmicClone>();
    ret->init(delay, plat, gjbgl);
    return ret;
}

CosmicPlayerObject* CosmicClone::getP1() const {
    return m_p1;
}

CosmicPlayerObject* CosmicClone::getP2() const {
    return m_p2;
}

int CosmicClone::getDelay() const {
    return m_delay;
}

void CosmicClone::checkCollision(PlayerObject* player) const {
    if (!player) return;
    auto rect1 = m_p1->getObjectRect();
    if (rect1.intersectsRect(player->getObjectRect())) {
        player->m_gameLayer->destroyPlayer(player, nullptr);
        playSFX(CosmicCloneSFXType::KillPlayer);
        player->m_maybeIsColliding = true; // for editor
    } else if (m_dual) {
        auto rect2 = m_p2->getObjectRect();
        if (rect2.intersectsRect(player->getObjectRect())) {
            player->m_gameLayer->destroyPlayer(player, nullptr);
            playSFX(CosmicCloneSFXType::KillPlayer);
            player->m_maybeIsColliding = true; // for editor
        }
    }
}

void CosmicClone::setDual(const bool dual) {
    m_p2->setVisible(dual);
    m_dual = dual;
}

void CosmicClone::setScale1(const float scale) {
    m_p1->setScale(scale * m_sillyScale);
}

void CosmicClone::setScale2(const float scale) {
    m_p2->setScale(scale * m_sillyScale);
}

void CosmicClone::setPos1(const cocos2d::CCPoint pos) {
    m_p1->setPosition({pos.x, pos.y + (15 * m_sillyScale - 15)});
}

void CosmicClone::setPos2(const cocos2d::CCPoint pos) {
    m_p2->setPosition({pos.x, pos.y + (15 * m_sillyScale - 15)});
}

IconType CosmicClone::getType(const int player) const {
    return player == 2 ? m_p2type : m_p1type;
}

void CosmicClone::setType(const IconType type, const int player) {
    auto gm = GameManager::get();
    if (player == 1) {
        if (type == m_p1type) return;
        m_p1type = type;
        switch (type) {
            case IconType::Cube:
                m_p1->toggleFlyMode(false, false);
                m_p1->toggleRollMode(false, false);
                m_p1->toggleBirdMode(false, false);
                m_p1->toggleDartMode(false, false);
                m_p1->toggleRobotMode(false, false);
                m_p1->toggleSpiderMode(false, false);
                m_p1->toggleSwingMode(false, false);
                m_p1->updatePlayerFrame(gm->getPlayerFrame());
                break;
            case IconType::Ship:
                m_p1->toggleFlyMode(true, true);
                if (m_p1->m_isPlatformer) {
                    m_p1->updatePlayerJetpackFrame(gm->getPlayerJetpack());
                } else {
                    m_p1->updatePlayerShipFrame(gm->getPlayerShip());
                }
                break;
            case IconType::Ball:
                m_p1->toggleRollMode(true, true);
                m_p1->updatePlayerRollFrame(gm->getPlayerBall());
                break;
            case IconType::Ufo:
                m_p1->toggleBirdMode(true, true);
                m_p1->updatePlayerBirdFrame(gm->getPlayerBird());
                break;
            case IconType::Wave:
                m_p1->toggleDartMode(true, true);
                m_p1->updatePlayerDartFrame(gm->getPlayerDart());
                break;
            case IconType::Robot:
                m_p1->toggleRobotMode(true, true);
                m_p1->updatePlayerRobotFrame(gm->getPlayerRobot());
                m_p1->m_robotBatchNode->setShaderProgram(m_p1->getShaderProgram());
                break;
            case IconType::Spider:
                m_p1->toggleSpiderMode(true, true);
                m_p1->updatePlayerSpiderFrame(gm->getPlayerSpider());
                m_p1->m_spiderBatchNode->setShaderProgram(m_p1->getShaderProgram());
                break;
            case IconType::Swing:
                m_p1->toggleSwingMode(true, true);
                m_p1->updatePlayerSwingFrame(gm->getPlayerSwing());
                break;
            default: break;
        }
        m_p1->updateGlowColor();
        updateShaderForPlayer(m_p1, m_p1->getShaderProgram(), true);
    } else if (player == 2) {
        if (type == m_p2type) return;
        m_p2type = type;
        auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        switch (type) {
            case IconType::Cube:
                m_p2->toggleFlyMode(false, false);
                m_p2->toggleRollMode(false, false);
                m_p2->toggleBirdMode(false, false);
                m_p2->toggleDartMode(false, false);
                m_p2->toggleRobotMode(false, false);
                m_p2->toggleSpiderMode(false, false);
                m_p2->toggleSwingMode(false, false);
                m_p2->updatePlayerFrame(sdi ? sdi->getSavedValue<int>("cube", gm->getPlayerFrame()) : gm->getPlayerFrame());
                break;
            case IconType::Ship:
                m_p2->toggleFlyMode(true, true);
                if (m_p2->m_isPlatformer) {
                    m_p2->updatePlayerJetpackFrame(sdi ? sdi->getSavedValue<int>("jetpack", gm->getPlayerJetpack()) : gm->getPlayerJetpack());
                } else {
                    m_p2->updatePlayerShipFrame(sdi ? sdi->getSavedValue<int>("ship", gm->getPlayerShip()) : gm->getPlayerShip());
                }
                break;
            case IconType::Ball:
                m_p2->toggleRollMode(true, true);
                m_p2->updatePlayerRollFrame(sdi ? sdi->getSavedValue<int>("roll", gm->getPlayerBall()) : gm->getPlayerBall());
                break;
            case IconType::Ufo:
                m_p2->toggleBirdMode(true, true);
                m_p2->updatePlayerBirdFrame(sdi ? sdi->getSavedValue<int>("bird", gm->getPlayerBird()) : gm->getPlayerBird());
                break;
            case IconType::Wave:
                m_p2->toggleDartMode(true, true);
                m_p2->updatePlayerDartFrame(sdi ? sdi->getSavedValue<int>("dart", gm->getPlayerDart()) : gm->getPlayerDart());
                break;
            case IconType::Robot:
                m_p2->toggleRobotMode(true, true);
                m_p2->updatePlayerRobotFrame(sdi ? sdi->getSavedValue<int>("robot", gm->getPlayerRobot()) : gm->getPlayerRobot());
                m_p2->m_spiderBatchNode->setShaderProgram(m_p2->getShaderProgram());                
                break;
            case IconType::Spider:
                m_p2->toggleSpiderMode(true, true);
                m_p2->updatePlayerSpiderFrame(sdi ? sdi->getSavedValue<int>("spider", gm->getPlayerSpider()) : gm->getPlayerSpider());
                m_p2->m_spiderBatchNode->setShaderProgram(m_p2->getShaderProgram());
                break;
            case IconType::Swing:
                m_p2->toggleSwingMode(true, true);
                m_p2->updatePlayerSwingFrame(sdi ? sdi->getSavedValue<int>("swing", gm->getPlayerSwing()) : gm->getPlayerSwing());
                break;
            default: break;
        }
        m_p1->updateGlowColor();
        updateShaderForPlayer(m_p2, m_p2->getShaderProgram(), true);
    }
    updateAnimation(player);
}

auto CosmicClone::getAnimation(const int player) const -> ::Animation {
    if (player == 1) return m_p1anim;
    return m_p2anim;
}

void CosmicClone::animate(const ::Animation animation, const int player) {
    if (player == 1) {
        if (m_p1anim == animation) return;
        m_p1anim = animation;
        updateAnimation(player);
    } else if (player == 2) {
        if (m_p2anim == animation) return;
        m_p2anim = animation;
        updateAnimation(player);
    }
}

void CosmicClone::updateAnimation(const int player) const {
    if (player == 1) {
        switch (m_p1anim) {
            case ::Animation::None: break;
            case ::Animation::Jump: {
                if (m_p1type == IconType::Cube) {
                    if (m_p1->m_isPlatformer) m_p1->animatePlatformerJump(1.f);
                } else if (m_p1type == IconType::Robot) {
                    m_p1->m_robotSprite->tweenToAnimation("jump_loop", 0.1f);
                } else if (m_p1type == IconType::Spider) {
                    m_p1->m_spiderSprite->tweenToAnimation("jump_loop", 0.1f);
                }
                break;
            }
            case ::Animation::Run: {
                if (m_p1type == IconType::Robot) {
                    m_p1->m_robotSprite->tweenToAnimation("run", 0.1f);
                } else if (m_p1type == IconType::Spider) {
                    m_p1->m_spiderSprite->tweenToAnimation("run", 0.1f);
                }
                break;
            }
            case ::Animation::Idle: {
                if (m_p1type == IconType::Robot) {
                    m_p1->m_robotSprite->tweenToAnimation("idle01", 0.1f);
                } else if (m_p1type == IconType::Spider) {
                    m_p1->m_spiderSprite->tweenToAnimation("idle01", 0.1f);
                }
                break;
            }
            case ::Animation::Fall: {
                if (m_p1type == IconType::Robot) {
                    m_p1->m_robotSprite->tweenToAnimation("fall_loop", 0.1f);
                } else if (m_p1type == IconType::Spider) {
                    m_p1->m_spiderSprite->tweenToAnimation("fall_loop", 0.1f);
                }
                break;
            }
        }
    } else if (player == 2) {
        switch (m_p2anim) {
            case ::Animation::None: break;
            case ::Animation::Jump: {
                if (m_p2type == IconType::Cube) {
                    if (m_p2->m_isPlatformer) m_p2->animatePlatformerJump(1.f);
                } else if (m_p2type == IconType::Robot) {
                    m_p2->m_robotSprite->tweenToAnimation("jump_loop", 0.1f);
                } else if (m_p2type == IconType::Spider) {
                    m_p2->m_spiderSprite->tweenToAnimation("jump_loop", 0.1f);
                }
                break;
            }
            case ::Animation::Run: {
                if (m_p2type == IconType::Robot) {
                    m_p2->m_robotSprite->tweenToAnimation("run", 0.1f);
                } else if (m_p2type == IconType::Spider) {
                    m_p2->m_spiderSprite->tweenToAnimation("run", 0.1f);
                }
            }
            case ::Animation::Idle: {
                if (m_p2type == IconType::Robot) {
                    m_p2->m_robotSprite->tweenToAnimation("idle01", 0.1f);
                } else if (m_p2type == IconType::Spider) {
                    m_p2->m_spiderSprite->tweenToAnimation("idle01", 0.1f);
                }
            }
            case ::Animation::Fall: {
                if (m_p2type == IconType::Robot) {
                    m_p2->m_robotSprite->tweenToAnimation("fall_loop", 0.1f);
                } else if (m_p2type == IconType::Spider) {
                    m_p2->m_spiderSprite->tweenToAnimation("fall_loop", 0.1f);
                }
            }
        }
    }
}

int CosmicClone::playSFX(CosmicCloneSFXType type) const {
    switch (type) {
        case CosmicCloneSFXType::FirstSpawn:
            if (m_style.type == "The Yellow One")
                return FMODAudioEngine::get()->playEffect("FordChimeSound.mp3"_spr);
            if (m_style.type == "Hungry Luma")
                return FMODAudioEngine::get()->playEffect("in.mp3"_spr);
            if (m_style.type == "Cosmic Clone\n(SMG 2)" || m_style.type == "Cosmic Mario\n(SMG 1)")
                return FMODAudioEngine::get()->playEffect("spawn.wav"_spr);
            if (m_style.type == "Badeline Chaser\n(Celeste)")
                return FMODAudioEngine::get()->playEffect("appear.wav"_spr, 1, 0, .35);
            if (m_style.type == "Markiplier")
                return FMODAudioEngine::get()->playEffect("hello everybody.mp3"_spr);
            return -1;
        case CosmicCloneSFXType::KillPlayer:
            if (m_style.type == "The Yellow One")
                return FMODAudioEngine::get()->playEffect("FordChimeSound.mp3"_spr);
        case CosmicCloneSFXType::Spawn:
            if (m_style.type == "Hungry Luma")
                return FMODAudioEngine::get()->playEffect("in.mp3"_spr);
            if (m_style.type == "The Yellow One")
                return FMODAudioEngine::get()->playEffect("FordChimeSound.mp3"_spr);
            if (m_style.type == "Badeline Chaser\n(Celeste)")
                return FMODAudioEngine::get()->playEffect("appear.wav"_spr, 1, 0, .35);
    
            return -1;
        case CosmicCloneSFXType::Die:
            if (m_style.type == "The Yellow One")
                return FMODAudioEngine::get()->playEffect("FordChimeSound.mp3"_spr);
            if (m_style.type == "Hungry Luma")
                return FMODAudioEngine::get()->playEffect("out.mp3"_spr);
            if (m_style.type == "Cosmic Clone\n(SMG 2)" || m_style.type == "Cosmic Mario\n(SMG 1)")
                return FMODAudioEngine::get()->playEffect("defeat.wav"_spr);
            if (m_style.type == "Badeline Chaser\n(Celeste)")
                return FMODAudioEngine::get()->playEffect("disappear.wav"_spr);
            return -1;
    }
    return -1;
}

void CosmicClone::updateStyle(Style style) {
    m_style = std::move(style);
    for (auto plr : {m_p1, m_p2}) {
        plr->setColor(m_style.getColor1());
        plr->setSecondColor(m_style.getColor2());
        if (m_style.useGlow) {
            plr->enableCustomGlowColor(m_style.getGlowColor());
            plr->m_hasGlow = true;
        } else {
            plr->disableCustomGlowColor();
            plr->m_hasGlow = true;
        }
        plr->updateGlowColor();
        if (m_style.type == "Cosmic Mario\n(SMG 1)") {
            if (!plr->m_gameLayer->m_isEditor) plr->toggleGhostEffect(GhostType::Disabled);
            updateShaderForPlayer(plr, ShaderManager::get().getCosmicShader());
            updateSpriteForPlayer(plr, nullptr);
        } else if (m_style.type == "Cosmic Clone\n(SMG 2)") {
            if (!plr->m_gameLayer->m_isEditor) plr->toggleGhostEffect(GhostType::Disabled);
            updateShaderForPlayer(plr, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
            updateSpriteForPlayer(plr, nullptr);
        } else if (m_style.type == "Badeline Chaser\n(Celeste)") {
            if (!plr->m_gameLayer->m_isEditor) {
                plr->toggleGhostEffect(GhostType::Enabled);
                if (auto trail = plr->m_ghostTrail) {
                    trail->m_color = ccColor3B{255, 0, 0};
                    trail->m_fadeInterval = .4f;
                }
            }
            updateShaderForPlayer(plr, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
            updateSpriteForPlayer(plr, nullptr);
        } else if (m_style.type == "Hungry Luma") {
            if (!plr->m_gameLayer->m_isEditor) plr->toggleGhostEffect(GhostType::Disabled);
            updateShaderForPlayer(plr, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
            updateSpriteForPlayer(plr, CCSprite::create("HungryLuma.png"_spr));
        } else if (m_style.type == "The Yellow One") {
            if (!plr->m_gameLayer->m_isEditor) plr->toggleGhostEffect(GhostType::Disabled);
            updateShaderForPlayer(plr, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
            updateSpriteForPlayer(plr, CCSprite::create("MD_DifficultyYOSmall.png"_spr));
        } else if (m_style.type == "eri") {
            if (!plr->m_gameLayer->m_isEditor) plr->toggleGhostEffect(GhostType::Disabled);
            updateShaderForPlayer(plr, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
            updateSpriteForPlayer(plr, CCSprite::create("eri.png"_spr), {1.27f, 1.f});
        } else if (m_style.type == "Markiplier") {
            if (!plr->m_gameLayer->m_isEditor) plr->toggleGhostEffect(GhostType::Disabled);
            updateShaderForPlayer(plr, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
            updateSpriteForPlayer(plr, CCSprite::create("markiplier.png"_spr));
        } else {
            if (!plr->m_gameLayer->m_isEditor) plr->toggleGhostEffect(GhostType::Disabled);
            updateShaderForPlayer(plr, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
            updateSpriteForPlayer(plr, nullptr);
        }
    }
}

void CosmicClone::remove() {
    if (m_p1) {
        removePlayer(m_p1);
        m_p1->removeFromParent();
        m_p1 = nullptr;
    }
    if (m_p2) {
        removePlayer(m_p2);
        m_p2->removeFromParent();
        m_p2 = nullptr;
    }
}

void CosmicClone::removePlayer(PlayerObject* player) {
    // Borrowed from Globed, thanks dank
    // https://github.com/GlobedGD/globed2/blob/5b79e2e613464915deb932e447e93536e7dcd7e1/src/core/game/VisualPlayer.cpp#L643
#define $clear(x) if (x) x->removeFromParent(); x = nullptr

    // Robtop does not properly remove most/all those nodes from the playerobject in the destructor,
    // so whenever someone leaves the level, these nodes are never deleted until you leave the level too.

    // Thanks sleepyut for finding this :)

    $clear(player->m_shipStreak);
    $clear(player->m_regularTrail);
    $clear(player->m_waveTrail);
    $clear(player->m_ghostTrail);

    $clear(player->m_playerGroundParticles);
    $clear(player->m_trailingParticles);
    $clear(player->m_shipClickParticles);
    $clear(player->m_vehicleGroundParticles);
    $clear(player->m_ufoClickParticles);
    $clear(player->m_robotBurstParticles);
    $clear(player->m_dashParticles);
    $clear(player->m_swingBurstParticles1);
    $clear(player->m_swingBurstParticles2);
    $clear(player->m_landParticles0);
    $clear(player->m_landParticles1);

#undef $clear
}
    
#define $apply(sprite, shader) if(sprite) { \
    sprite->setShaderProgram(shader); \
} else { \
    log::error("Could not find {}.", #sprite); \
}

inline void recurseApply(CCNode* node, CCGLProgram* shader) {
    $apply(node, shader);
    for (auto child : node->getChildrenExt()) {
        recurseApply(child, shader);
    }
}

#undef $apply

void CosmicClone::updateShaderForPlayer(PlayerObject* player, CCGLProgram* shader, bool applyToRobotSprites) {
    if (!player) return log::error("Cannot set a shader for a nullptr PlayerObject.");
    if (!shader) return log::error("Cannot set a nullptr shader for a PlayerObject.");
    recurseApply(player, shader);
}

void CosmicClone::updateShaderForPlayer(SimplePlayer* player, CCGLProgram* shader, bool applyToRobotSprites) {
    if (!player) return log::error("Cannot set a shader for a nullptr PlayerObject.");
    if (!shader) return log::error("Cannot set a nullptr shader for a PlayerObject.");
    recurseApply(player, shader);
}


void CosmicClone::updateSpriteForPlayer(PlayerObject* player, CCSprite* sprite, const CCPoint& scaleMult) {
    if (!player) return log::error("Cannot set a sprite for a nullptr PlayerObject.");

    if (auto c = player->getChildByID("display"_spr)) c->removeFromParent();

    if (sprite) {
        sprite->setID("display"_spr);
        sprite->setScaleX(30 / sprite->getContentWidth() * scaleMult.x);
        sprite->setScaleY(30 / sprite->getContentHeight() * scaleMult.y);
        player->addChildAtPosition(sprite, Anchor::Center, {0, 0}, false);
    }

    player->m_mainLayer->setVisible(sprite == nullptr);
}

void CosmicClone::updateSpriteForPlayer(SimplePlayer* player, CCSprite* sprite, const CCPoint& scaleMult) {
    if (!player) return log::error("Cannot set a sprite for a nullptr PlayerObject.");

    if (auto c = player->getChildByID("display"_spr)) c->removeFromParent();

    if (sprite) {
        sprite->setID("display"_spr);
        sprite->setScaleX(30 / sprite->getContentWidth() * scaleMult.x);
        sprite->setScaleY(30 / sprite->getContentHeight() * scaleMult.y);
        player->addChildAtPosition(sprite, Anchor::Center, {0, 0}, false);
    }

    player->getChildByIndex(0)->setVisible(sprite == nullptr);
}
