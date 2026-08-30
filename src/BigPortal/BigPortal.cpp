#include "BigPortal.hpp"

#include "../GJBaseGameLayer.cpp"

// constants

const ccColor3B BigPortal::EFFECT_COLOR = ccc3(0, 128, 255);

const std::string BigPortal::FRAME_BACK = "big-portal-back.png"_spr;

const std::string BigPortal::FRAME_FRONT = "big-portal-front.png"_spr;

const float BigPortal::PLAYER_SIZE = 1.6f;

const std::string BigPortal::STRING_ID = "big-portal"_spr;

// functions

BigPortal* BigPortal::create(ObjectInfo* info) {
    // ObjectAPI will internally handle auto releasing and calling init.
    return new BigPortal(info);
}

PopupOptions BigPortal::getEditSpecialConfig(const Selected& selected) {
    return PopupConfig::builder()
        .width(200)
        .height(140)
        .title("Big Portal")
        .menu(AxisLayoutMenu::builder()
                  .padding(Padding(0.f, 12.5f, 0.f, 7.5f))
                  .menu(ToggleMenu::builder()
                            .id("no-multi-activate"_spr)
                            .title("No Multi\nActivate")
                            .currentValue([](const Selected& selected, Popup* popup) {
                                return getCommonValueOrDefault(selected,
                                                               &BigPortal::m_isNoMultiActivate);
                            })
                            .onValue([](const bool value, const Selected& selected, Popup* popup) {
                                applyValueToSelected(selected, &BigPortal::m_isNoMultiActivate,
                                                     value);
                            })
                            .build())
                  .build())
        .build();
}

void BigPortal::postPlayLayerInit() {
    createBackFrame(FRAME_BACK, CCPoint{});

    if (!m_hasNoParticles) {
        if (auto particles = createAndAddParticle(6, "portalEffect08.plist", 4,
                                                  tCCPositionType::kCCPositionTypeGrouped)) {
            particles->setStartColor(ccc4FFromccc3B(BigPortal::EFFECT_COLOR));
            particles->setStartColorVar(ccc4f(.25f, .25f, 0.f, .5f));
            particles->setEndColor(ccc4FFromccc3B(BigPortal::EFFECT_COLOR));
            claimParticle();
        }
    }
}

void BigPortal::setPlayerSize(PlayerObject* player, float s) {
    player->m_vehicleSize = s;
    player->m_spriteWidthScale = s;
    player->m_spriteHeightScale = s;

    // see PlayerObject::togglePlayerScale
    player->m_landParticles0->loadScaledDefaults(s);
    player->m_landParticles1->loadScaledDefaults(s);
    player->m_playerGroundParticles->loadScaledDefaults(s);
    player->m_vehicleGroundParticles->loadScaledDefaults(s);
    player->m_trailingParticles->loadScaledDefaults(s);
    player->m_shipClickParticles->loadScaledDefaults(s);
    player->m_ufoClickParticles->loadScaledDefaults(s);
    player->m_robotBurstParticles->loadScaledDefaults(s);
    player->m_dashParticles->loadScaledDefaults(s);
    player->m_swingBurstParticles1->loadScaledDefaults(s);
    player->m_swingBurstParticles2->loadScaledDefaults(s);
    player->m_regularTrail->setStroke(player->m_streakStrokeWidth * s *
                                      (player->m_isDart ? .8f : 1.f));
    if (player->m_playEffects || player->m_isDart) {
        player->m_waveTrail->m_waveSize = s;
    }
    if (player->m_ghostTrail) {
        player->m_ghostTrail->m_playerScale = s;
    }
    if (player->m_isBall && player->m_isRotating && !player->m_isLocked && !player->m_isDashing) {
        player->m_isRotating = false;
        player->m_isBallRotating2 = false;
        player->m_isBallRotating = false;
        player->m_rotationSpeed = 0.0;
        // LATER contains m_vehicleSize
        player->runBallRotation(1.0);
    }
    player->placeStreakPoint();
    player->updateRobotAnimationSpeed();  // LATER contains m_vehicleSize
}

void BigPortal::runScaleAction(PlayerObject* player) {
    player->m_actionManager->m_internalActions->removeObjectForKey(6);
    auto scaleTo = CCScaleTo::create(.5f, PLAYER_SIZE);
    auto ease = CCEaseElasticOut::create(scaleTo);
    ease->setTag(6);
    player->m_actionManager->m_internalActions->setObject(ease, ease->getTag());
    player->runAction(ease);
}

void BigPortal::runGrowToBigEffects(PlayerObject* player) {
    // lightning
    if (player->m_lastActivatedPortal) {
        if (auto baseLayer = GJBaseGameLayer::get()) {
            baseLayer->lightningFlash(player->m_lastActivatedPortal->getPosition(), EFFECT_COLOR);
        }
    }

    // portal circle
    player->spawnPortalCircle(EFFECT_COLOR, 45.f);

    // scale circle (PlayerObject::spawnScaleCircle)
    // follows same ratio as green portal
    auto wave = CCCircleWave::create(10.f, PLAYER_SIZE * 40.f, .3f, false, true);
    wave->followObject(player, true);
    wave->m_color = EFFECT_COLOR;
    if (auto parentLayer = player->getParent()) {
        parentLayer->addChild(wave);
    }
    if (auto playLayer = PlayLayer::get()) {
        playLayer->m_circleWaveArray->addObject(wave);
    }
}

void BigPortal::runShrinkToRegularEffects(PlayerObject* player) {
    const ccColor3B color = ccc3(0, 255, 150);

    // lightning
    if (player->m_lastActivatedPortal) {
        if (auto baseLayer = GJBaseGameLayer::get()) {
            baseLayer->lightningFlash(player->m_lastActivatedPortal->getPosition(), color);
        }
    }

    // portal circle
    player->spawnPortalCircle(color, 45.f);

    // scale circle
    // the ratio for the pink portal should be PLAYER_SIZE * 50.f but it looks bad
    auto wave = CCCircleWave::create(PLAYER_SIZE * 40.f, 2.f, .25f, true, true);
    wave->followObject(player, true);
    wave->m_color = color;
    if (auto parentLayer = player->getParent()) {
        parentLayer->addChild(wave);
    }
    if (auto playLayer = PlayLayer::get()) {
        playLayer->m_circleWaveArray->addObject(wave);
    }

    // for green portal:
    // color = ccc3(0, 255, 150)
    // duration = 0.3
    // radius = 10 -> 40

    // for pink portal:
    // color = ccc3(255, 0, 150)
    // duration = 0.25
    // radius = 50 -> 2
}
