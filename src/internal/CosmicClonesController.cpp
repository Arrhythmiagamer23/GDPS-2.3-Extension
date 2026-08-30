#include "CosmicClonesController.hpp"

#include <arc/time/Sleep.hpp>

#include "../Utils.hpp"
#include "../hooks/Gameplay.hpp"

using namespace geode::prelude;

bool CosmicClonesController::start() {
    if (!m_stopped) {
        log::error("Could not start cosmic clones controller: this controller is already running!");
        return false;
    }
    if (!m_bgl) {
        log::error("Could not start cosmic clones controller: GJBGL doesn't exist!");
        return false;
    }
    log::dev("Starting trigger!");

    m_stopped = false;
    m_startOffset = m_bgl->m_gameState.m_currentProgress;
    return true;
}

void CosmicClonesController::stop(bool immediate) {
    if (m_stopped) return log::error("Could not stop cosmic clones controller: this controller is already stopped!");
    log::dev("Stopping trigger!");
    if (!immediate) {
        bool hasSfxed = false;
        for (const auto& clone : m_clones) {
            if (!hasSfxed) {
                hasSfxed = true;
                if (m_sfx) (void) clone->playSFX(CosmicCloneSFXType::Die);
            }
            clone->getP1()->playerDestroyed(false);
            clone->getP2()->playerDestroyed(false);
        }
        async::spawn(arc::sleep(asp::Duration::fromSecs(1)), [this] {
            cleanup();
        });
    } else {
        cleanup();
    }
    m_stopped = true;
}

void CosmicClonesController::loadConfigFromSettings() {
    m_initialDelay = getSettingFast<"spawn-delay", float>();
    m_delay = getSettingFast<"delay", float>();
    m_damage = !getSettingFast<"friendly", bool>();
    m_count = getSettingFast<"clones", int>();
    m_styles = getSettingFast<"styles", std::vector<Style>>();
    m_sfx = getSettingFast<"sfx", bool>();
}

void CosmicClonesController::loadConfigFromTrigger(const CosmicClonesTrigger* trigger) {
    if (!trigger) return log::error("Could not load config from trigger: trigger doesn't exist!");
    m_initialDelay = trigger->m_initialDelay;
    m_delay = trigger->m_delay;
    m_damage = trigger->m_damage;
    m_count = trigger->m_count;
    m_styles = trigger->m_styles;
    m_sfx = trigger->m_sfx;
}

void CosmicClonesController::loadConfigFromTrigger() {
    loadConfigFromTrigger(m_trigger);
}

unsigned int CosmicClonesController::getOffset() const {
    return m_startOffset;
}

bool CosmicClonesController::isStopped() const {
    return m_stopped;
}

std::vector<std::shared_ptr<CosmicClone>> CosmicClonesController::getClones() const {
    return m_clones;
}

void CosmicClonesController::cleanup() {
    for (const auto& clone : m_clones) {
        clone->remove();
    }
    m_clones.clear();
    m_snapshots.clear();
    for (const auto channel : m_sfxIds) {
        FMODAudioEngine::get()->stopChannel(channel);
    }
    m_sfxIds.clear();
}

void CosmicClonesController::tick(int prog) {
    if (m_stopped || m_bgl->m_playerDied) return;
    auto bglFields = m_bgl->m_fields.self();
    int tick = prog - m_startOffset;
    for (const auto& clone : m_clones) {
        auto del = tick - clone->getDelay();
        if (m_snapshots.contains(del)) {
            auto snap = m_snapshots.at(del);
            clone->setDual(snap.dualEnabled);
            auto p1 = clone->getP1();
            clone->setPos1(snap.pos);
            clone->setScale1(snap.scale);
            p1->setRotation(snap.rotation);
            p1->setVisible(snap.visible);
            p1->m_isGoingLeft = snap.left;
            p1->m_mainLayer->setScaleX(snap.left ? -1 : 1);
            p1->m_isSideways = snap.side;
            p1->flipGravity(snap.flip, true);
            clone->setType(snap.type, 1);
            clone->animate(snap.anim, 1);
            if (snap.dualEnabled) {
                auto p2 = clone->getP2();
                clone->setPos2(snap.pos2);
                clone->setScale2(snap.scale2);
                p2->setRotation(snap.rotation2);
                p2->setVisible(snap.visible2);
                p2->m_isGoingLeft = snap.left2;
                p2->m_mainLayer->setScaleX(snap.left2 ? -1 : 1);
                p2->m_isSideways = snap.side2;
                p2->flipGravity(snap.flip2, true);
                clone->setType(snap.type2, 2);
                clone->animate(snap.anim2, 2);
            }
        }
    }
    const auto& styles = m_styles;
    for (int i = 1; i <= m_count; i++) {
        auto del = std::round(((i - 1) * m_delay + m_initialDelay) * 120) * 2;
        if (tick == del) {
            auto clone = CosmicClone::create(del, m_bgl->m_isPlatformer, m_bgl);
            clone->updateStyle(styles[(i - 1) % styles.size()]);
            m_bgl->m_objectLayer->addChild(clone->getP1());
            m_bgl->m_objectLayer->addChild(clone->getP2());
            clone->getP1()->setZOrder(m_bgl->m_player1->getZOrder());
            clone->getP2()->setZOrder(m_bgl->m_player2->getZOrder());
            m_clones.push_back(clone);
            if (m_sfx) {
                auto id = clone->playSFX(i > 1 ? CosmicCloneSFXType::Spawn : CosmicCloneSFXType::FirstSpawn);
                if (id > -1) m_sfxIds.push_back(id);
            }
        }
    }
    int max = tick - ((m_count - 1) * m_delay + m_initialDelay) * 240;
    if (!m_bgl->m_isPracticeMode && m_snapshots.contains(max)) {
        m_snapshots.erase(max);
    }
    auto snap = Snapshot{
        .pos = m_bgl->m_player1->m_position,
        .rotation = m_bgl->m_player1->getRotation(),
        .scale = m_bgl->m_player1->getScale(),
        .left = m_bgl->m_player1->m_isGoingLeft,
        .side = m_bgl->m_player1->m_isSideways,
        .visible = m_bgl->m_player1->isVisible(),
        .flip = m_bgl->m_player1->m_isUpsideDown,
    };
    if (m_bgl->m_player1->m_isShip) {
        snap.type = IconType::Ship;
    } else if (m_bgl->m_player1->m_isBall) {
        snap.type = IconType::Ball;
    } else if (m_bgl->m_player1->m_isBird) {
        snap.type = IconType::Ufo;
    } else if (m_bgl->m_player1->m_isDart) {
        snap.type = IconType::Wave;
    } else if (m_bgl->m_player1->m_isRobot) {
        snap.type = IconType::Robot;
    } else if (m_bgl->m_player1->m_isSpider) {
        snap.type = IconType::Spider;
    } else if (m_bgl->m_player1->m_isSwing) {
        snap.type = IconType::Swing;
    } else {
        snap.type = IconType::Cube;
    }
    bool move = m_bgl->m_player1->m_platformerXVelocity > 0.1 || m_bgl->m_player1->m_platformerXVelocity < -0.1;
    int vel = m_bgl->m_player1->m_yVelocity * (m_bgl->m_player1->m_isUpsideDown ? -1 : 1);
    if (!m_bgl->m_player1->m_isOnGround && vel < -0.1) {
        snap.anim = ::Animation::Fall;
    } else if (m_bgl->m_player1->m_isOnGround && move) {
        snap.anim = ::Animation::Run;
    } else if (m_bgl->m_player1->m_isOnGround && !move) {
        snap.anim = ::Animation::Idle;
    } else if (!m_bgl->m_player1->m_isOnGround && vel > 0.1 && bglFields->m_p1Jump) {
        bglFields->m_p1Jump = false;
        snap.anim = ::Animation::Jump;
    }

    if (m_bgl->m_gameState.m_isDualMode) {
        snap.dualEnabled = true;
        snap.pos2 = m_bgl->m_player2->getPosition();
        snap.rotation2 = m_bgl->m_player2->getRotation();
        snap.scale2 = m_bgl->m_player2->getScale();
        snap.left2 = m_bgl->m_player2->m_isGoingLeft;
        snap.side2 = m_bgl->m_player2->m_isSideways;
        snap.flip2 = m_bgl->m_player2->m_isUpsideDown;
        snap.visible2 = m_bgl->m_player2->isVisible();
        if (m_bgl->m_player2->m_isShip) {
            snap.type2 = IconType::Ship;
        } else if (m_bgl->m_player2->m_isBall) {
            snap.type2 = IconType::Ball;
        } else if (m_bgl->m_player2->m_isBird) {
            snap.type2 = IconType::Ufo;
        } else if (m_bgl->m_player2->m_isDart) {
            snap.type2 = IconType::Wave;
        } else if (m_bgl->m_player2->m_isRobot) {
            snap.type2 = IconType::Robot;
        } else if (m_bgl->m_player2->m_isSpider) {
            snap.type2 = IconType::Spider;
        } else if (m_bgl->m_player2->m_isSwing) {
            snap.type2 = IconType::Swing;
        } else {
            snap.type2 = IconType::Cube;
        }
    }
    m_snapshots[tick] = std::move(snap);

    if (!m_damage) return;
    if (!bglFields->m_p1Frozen && bglFields->m_p1Immunity > 0) {
        bglFields->m_p1Immunity--;
    } else if (!bglFields->m_p1Frozen) {
        for (auto const& clone : m_clones) {
            if (clone->getDelay() <= tick) clone->checkCollision(m_bgl->m_player1);
        }
    }
    if (!bglFields->m_p2Frozen && bglFields->m_p2Immunity > 0) {
        bglFields->m_p2Immunity--;
    } else if (!bglFields->m_p2Frozen && m_bgl->m_gameState.m_isDualMode && m_bgl->m_player2 != nullptr) {
        for (auto const& clone : m_clones) {
            if (clone->getDelay() <= tick) clone->checkCollision(m_bgl->m_player2);
        }
    }
}

void CosmicClonesController::softReset(int prog) {
    for (auto clone = m_clones.begin(); clone != m_clones.end();) {
        if (clone->get()->getDelay() > prog - m_startOffset) {
            clone->get()->remove();
            m_clones.erase(clone);
        } else {
            ++clone;
        }
    }
    std::erase_if(m_snapshots, [this, prog](const std::pair<const int, Snapshot>& snap) {
	    return snap.first > prog - m_startOffset;
    });
}

std::shared_ptr<CosmicClonesController> CosmicClonesController::createWithSettings(CosmicClonesGJBGL* bgl) {
    auto ret = std::make_shared<CosmicClonesController>(bgl);
    ret->loadConfigFromSettings();
    return ret;
}

std::shared_ptr<CosmicClonesController> CosmicClonesController::createFromTrigger(CosmicClonesGJBGL* bgl, CosmicClonesTrigger* trigger) {
    auto ret = std::make_shared<CosmicClonesController>(bgl, trigger);
    ret->loadConfigFromTrigger();
    return ret;
}

CosmicClonesController::CosmicClonesController(CosmicClonesGJBGL* bgl) : m_bgl(bgl) {}
CosmicClonesController::CosmicClonesController(CosmicClonesGJBGL* bgl, CosmicClonesTrigger* trigger) : m_bgl(bgl), m_trigger(trigger) {}

CosmicClonesController::~CosmicClonesController() {
    if (!m_stopped) cleanup();
}
