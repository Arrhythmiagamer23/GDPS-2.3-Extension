#include <Geode/Geode.hpp>

#include "BigPortal/BigPortal.hpp"
#include "PlayerCheckpoint.cpp"

using namespace geode::prelude;

#include <Geode/modify/PlayerObject.hpp>
class $modify(MyPlayerObject, PlayerObject) {
    static void onModify(auto& self) {
        if (!self.setHookPriorityPre("PlayerObject::propellPlayer", Priority::Last)) {
            log::warn("Failed to set hook priority for propellPlayer!");
        }

        if (!self.setHookPriorityPre("PlayerObject::ringJump", Priority::Last)) {
            log::warn("Failed to set hook priority for ringJump!");
        }

        if (!self.setHookPriorityPre("PlayerObject::updateJump", Priority::Last)) {
            log::warn("Failed to set hook priority for updateJump!");
        }
    }

    // ------------------------------

    bool isBig(float s) { return s > 1.f; }

    bool isBig() { return isBig(m_vehicleSize); }

    // ------------------------------

    void runNormalRotation(bool notNormalMode, float speed) {
        if (!isBig()) {
            PlayerObject::runNormalRotation(notNormalMode, speed);
            return;
        }

        if (notNormalMode ||
            // is cube
            (!m_isShip && !m_isBird && !m_isDart && !m_isSwing && !m_isRobot && !m_isSpider &&
             !m_isDashing &&
             // has xv
             (!m_isPlatformer || m_holdingLeft || m_holdingRight || m_platformerMovingLeft ||
              m_platformerMovingRight))) {
            // CHANGED - cube rotation speed
            // normal = 13/30, mini = 10/30, big = 16/30
            const float rotationDiv = 0.533f;
            m_isRotating = true;
            m_rotationSpeed = 180 * flipMod() * (m_isGoingLeft ? -1 : 1) * (m_isSideways ? -1 : 1) *
                              m_gravityMod * speed / rotationDiv;
        }
    }

    void loadFromCheckpoint(PlayerCheckpoint* object) {
        PlayerObject::loadFromCheckpoint(object);

        if (auto checkpoint = static_cast<MyPlayerCheckpoint*>(object)) {
            if (isBig(checkpoint->m_fields->m_vehicleSize)) {
                BigPortal::setPlayerSize(this, checkpoint->m_fields->m_vehicleSize);
                updatePlayerScale();
            }
        }
    }

    void propellPlayer(float yVelocity, bool noEffects, int objectTypeInt) {
        if (!isBig()) {
            PlayerObject::propellPlayer(yVelocity, noEffects, objectTypeInt);
            return;
        }

        auto objectType = static_cast<GameObjectType>(objectTypeInt);

        if (!(std::unordered_set{
                GameObjectType::YellowJumpPad,
                GameObjectType::PinkJumpPad,
                GameObjectType::RedJumpPad,
            }
                  .contains(objectType))) {
            PlayerObject::propellPlayer(yVelocity, noEffects, objectTypeInt);
            return;
        }

        // reverse engineering velocity
        float newVelocity = yVelocity;

        // velocity at normal size
        if (m_vehicleSize != 1.f) {
            newVelocity /= .8f;
        }

        // forward engineering velocity (at big size)
        // CHANGED - pad velocity
        newVelocity *= 1.1f;

        // red pad gamemode specific changes, similar to GJBaseGameLayer::bumpPlayer
        if (objectType == GameObjectType::RedJumpPad) {
            if (m_isShip) {
                // CHANGED - ship red pad velocity
                newVelocity *= .8f;
            } else if (m_isBird) {
                // CHANGED - ufo red pad velocity
                newVelocity *= .78f;
            }
        }

        PlayerObject::propellPlayer(newVelocity, noEffects, objectTypeInt);
    }

    void ringJump(RingObject* object, bool skipCheck) {
        if (!isBig()) {
            PlayerObject::ringJump(object, skipCheck);
            return;
        }

        if (!object) {
            return;
        }

        auto objectType = object->getType();

        if (!(std::unordered_set{
                GameObjectType::YellowJumpRing,
                GameObjectType::PinkJumpRing,
                GameObjectType::GravityRing,
                GameObjectType::GreenRing,
                GameObjectType::DropRing,
                GameObjectType::RedJumpRing,
            }
                  .contains(objectType))) {
            PlayerObject::ringJump(object, skipCheck);
            return;
        }

        if (m_isDead || !m_stateRingJump2 || m_isDashing || !m_stateJumpBuffered || m_touchedRing) {
            return;
        }

        if (m_ringRelatedSet.find(object->m_uniqueID) != m_ringRelatedSet.end()) {
            return;
        }

        bool hasNoEffects = object->m_hasNoEffects;

        if (object->m_isReverse) {
            reversePlayer(object);
        }

        m_ringJumpRelated = true;
        m_ringRelatedSet.insert(object->m_uniqueID);

        if (m_gameLayer) {
            m_gameLayer->gameEventTriggered(GJGameEvent::OrbActivated, 0, m_uniqueID);
            m_gameLayer->gameEventTriggered(m_gameLayer->objectTypeToGameEvent((int)objectType), 0,
                                            m_uniqueID);
        }

        m_touchedRing = true;
        m_touchingRings->removeObject(object);
        m_padRingRelated = true;
        m_stateRingJump = false;

        // black ring
        if (objectType == GameObjectType::DropRing) {
            float vel;
            if (m_isShip || m_isBird || m_isDart || m_isSwing) {
                vel = -14.f * (m_isBird ? .8f : 1.f);
            } else {
                vel = -15.f * (m_isSpider ? 1.1f : 1.f);
            }
            setYVelocity(vel * flipMod(), 0);

            if (m_isBall) {
                runBallRotation2();
            } else if (!m_isLocked && !m_isDashing) {
                m_isRotating = false;
                m_isBallRotating2 = false;
                m_isBallRotating = false;
                m_rotationSpeed = 0.f;
                runNormalRotation(false, 1.f);
            }

            activateStreak();
            m_hasEverHitRing = true;
            m_isAccelerating = true;

            if (m_isBall || m_isSwing) {
                m_jumpBuffered = false;
            }
        }

        // yellow, blue, pink, red, green ring
        else {
            m_maybeIsBoosted = true;
            m_isOnGround2 = false;
            m_isOnGround = false;

            // CHANGED - ring velocity
            // const float sizeScale = m_vehicleSize == 1.f ? 1.f : .8f;
            const float sizeScale = 1.2f;

            // uhh why is it called yStart lmao
            float jumpVel = m_yStart;

            switch (objectType) {
                case GameObjectType::GravityRing:
                    jumpVel *= .8f;
                    break;

                case GameObjectType::GreenRing:
                    if (m_isShip) {
                        jumpVel *= .7f;
                    }
                    break;

                case GameObjectType::PinkJumpRing:
                    if (m_isShip) {
                        jumpVel *= .37f;
                    } else if (m_isBird) {
                        jumpVel *= .42f;
                    } else if (m_isBall) {
                        jumpVel *= .77f;
                    } else {
                        jumpVel *= .72f;
                    }
                    break;

                case GameObjectType::RedJumpRing:
                    if (m_isShip) {
                        // CHANGED - ship red ring velocity
                        // jumpVel *= m_vehicleSize == 1.f ? 1.f : 1.4f;
                        jumpVel *= .75f;
                    } else if (m_isBird) {
                        // CHANGED - ufo red ring velocity
                        // jumpVel *= m_vehicleSize == 1.f ? 1.02f : 1.36f;
                        jumpVel *= .79f;
                    } else if (m_isBall) {
                        jumpVel *= 1.34f;
                    } else if (m_isRobot) {
                        jumpVel *= 1.28f;
                    } else if (m_isSpider) {
                        jumpVel *= 1.34f;
                    } else {
                        jumpVel *= 1.38f;
                    }
                    break;

                default:
                    if (m_isRobot) {
                        jumpVel *= .9f;
                    }
                    break;
            }

            if (objectType == GameObjectType::GreenRing) {
                ringJumpFlipGravity(object, hasNoEffects);
            }

            int sign = flipMod();
            setYVelocity(sign * jumpVel * sizeScale, 0);

            if (m_isBall) {
                runBallRotation2();
            } else if (!m_isLocked && !m_isDashing) {
                m_isRotating = false;
                m_isBallRotating2 = false;
                m_isBallRotating = false;
                m_rotationSpeed = 0.f;
                runNormalRotation(false, 1.f);
            }

            if (m_isPlatformer && !m_isRotating) {
                animatePlatformerJump(1.f);
            }

            m_lastGroundedPos = getPosition();
            activateStreak();
            m_hasEverHitRing = true;

            if (m_isBall || m_isSpider || m_isSwing) {
                m_yVelocity *= m_isSwing ? .6 : .7;
                m_jumpBuffered = false;
            }

            if (objectType == GameObjectType::GravityRing) {
                ringJumpFlipGravity(object, hasNoEffects);
            }

            // jump animations
            if (m_isRobot) {
                m_robotSprite->runAnimation("jump_start");
            } else if (m_isSpider) {
                m_spiderSprite->m_paSprite->stopAllActions();
                m_spiderSprite->stopActionByTag(1);
                m_spiderSprite->m_animationManager->stopAnimations();
                m_spiderSprite->tweenToAnimation("jump", .06f);
            }

            // red ring sets the sustained boost flag like a robot jump
            if (objectType == GameObjectType::RedJumpRing) {
                m_isAccelerating = true;
            }
        }

        // circle wave
        auto gm = GameManager::get();
        if (gm && gm->m_playLayer &&
            !gm->m_performanceMode
            // hasBeenActivated is vfunction174
            && !m_maybeReducedEffects && !object->hasBeenActivated() && m_playEffects &&
            !hasNoEffects) {
            auto playLayer = gm->m_playLayer;

            ccColor3B color;

            switch (objectType) {
                case GameObjectType::PinkJumpRing:
                    color = ccc3(255, 0, 255);
                    break;

                case GameObjectType::GravityRing:
                    color = ccc3(0, 255, 255);
                    break;

                case GameObjectType::GreenRing:
                    color = ccc3(0, 255, 0);
                    break;

                case GameObjectType::DropRing: {
                    if (auto colorSprite = playLayer->m_effectManager->getColorSprite(0x3ef)) {
                        color = colorSprite->m_copyColor;
                    } else {
                        color = ccc3(255, 255, 255);
                    }
                    break;
                }

                case GameObjectType::RedJumpRing:
                    color = ccc3(255, 100, 0);
                    break;

                default:
                    color = ccc3(200, 255, 0);
            }

            auto wave = CCCircleWave::create(
                objectType == GameObjectType::RedJumpRing ? 42.f : 35.f, 5.f, .35f, true, true);
            wave->followObject(object, true);
            wave->m_color = color;

            m_parentLayer->addChild(wave, 0);
            playLayer->m_circleWaveArray->addObject(wave);
        }

        if (!m_editorEnabled) {
            object->playTriggerEffect();
        }

        object->activatedByPlayer(this);  // vfunction172
        object->powerOffObject();         // vfunction209
        incrementJumps();
    }

    void ringJumpFlipGravity(RingObject* object, bool hasNoEffects) {
        if (m_playEffects) {
            if (auto playLayer = PlayLayer::get()) {
                playLayer->flipGravity(this, !m_isUpsideDown, true);
                if (!hasNoEffects) {
                    // vfunction161
                    playLayer->playGravityEffect(m_isUpsideDown);
                }
            }
        } else if (auto editor = LevelEditorLayer::get()) {
            editor->flipGravity(this, !m_isUpsideDown, true);
        } else {
            flipGravity(!m_isUpsideDown, true);
        }
    }

    void saveToCheckpoint(PlayerCheckpoint* object) {
        PlayerObject::saveToCheckpoint(object);

        if (auto checkpoint = static_cast<MyPlayerCheckpoint*>(object)) {
            checkpoint->m_fields->m_vehicleSize = m_vehicleSize;
        }
    }

    void togglePlayerScale(bool enable, bool noEffects) {
        // mini/regular/big -> big
        CUSTOM_OBJECT_IMPLEMENT(m_lastActivatedPortal, BigPortal, _, {
            if (m_vehicleSize == BigPortal::PLAYER_SIZE) {
                return;
            }

            BigPortal::setPlayerSize(this, BigPortal::PLAYER_SIZE);

            auto playLayer = PlayLayer::get();
            if (m_playEffects && !m_maybeReducedEffects && playLayer &&
                !playLayer->m_skipArtReload) {
                BigPortal::runScaleAction(this);
                if (!noEffects) {
                    BigPortal::runGrowToBigEffects(this);
                }
            } else {
                updatePlayerScale();
            }
            return;
        });

        // big -> mini/regular
        if (isBig()) {
            // pretend it's the opposite size to make the super call work
            m_vehicleSize = enable ? 1.f : .6f;

            // shrink in green portal
            if (!enable) {
                PlayerObject::togglePlayerScale(enable, true);

                auto playLayer = PlayLayer::get();
                if (m_playEffects && !m_maybeReducedEffects && playLayer &&
                    !playLayer->m_skipArtReload && !noEffects) {
                    BigPortal::runShrinkToRegularEffects(this);
                }
            }

            // shrink in mini portal
            else {
                PlayerObject::togglePlayerScale(enable, noEffects);
            }
        }

        // mini/regular -> mini/regular (vanilla)
        else {
            PlayerObject::togglePlayerScale(enable, noEffects);
        }
    }

    void update(float dt) {
        PlayerObject::update(dt);

        // wave velocity
        if (isBig() && m_isDart && !m_isDashing && !m_isLocked) {
            CCPoint pos = getPosition();

            // i see why we can't have wave in platformer 😔
            const double speed = m_playerSpeed * m_speedMultiplier;
            const int sign = flipMod() * (m_jumpBuffered ? 1 : -1);

            // CHANGED wave velocity
            // 0x = same as mini wave
            // 1x = same as normal wave
            pos.y -= 1.25f * abs(speed) * dt * sign;

            setPosition(pos);
        }
    }

    void updateJump(float dt) {
        if (!isBig()) {
            PlayerObject::updateJump(dt);
            return;
        }

        bool holdingLeft = m_holdingLeft;
        bool holdingRight = m_holdingRight;
        if (holdingLeft && holdingRight) {
            if (m_leftPressedFirst) {
                holdingRight = false;
            } else {
                holdingLeft = false;
            }
        }

        if (m_isPlatformer && m_isOnSlope) {
            float slopeDegrees = abs(m_slopeAngleRadians * 57.29578f);
            if (slopeDegrees >= 80.f || (slopeDegrees >= 40.f && m_currentSlope->m_isIceBlock)) {
                m_isOnGround = false;
            }
        }

        float gravity =
            m_gravityMod * (m_isBall || m_isShip || m_isBird || m_isDart || m_isSwing || m_isSpider
                                ? .958199f
                                : m_gravity);

        // CHANGED - jump velocity
        // const float sizeScale = m_vehicleSize == 1.f ? 1.f : .8f;
        const float sizeScale = 1.2f;

        int sign = flipMod();

        // FLYING
        if (m_isShip || m_isBird || m_isDart || m_isSwing) {
            // CHANGED - flying velocity
            // const float flightSizeScale = m_vehicleSize == 1.f ? sizeScale : .85f;
            const float flightSizeScale = 1.15f;

            if (
                // lower limit
                m_yVelocity >= (m_isUpsideDown ? -8.f : -6.4f) / flightSizeScale
                // upper limit
                && m_yVelocity <= (m_isUpsideDown ? 6.4f : 8.f) / flightSizeScale) {
                m_isAccelerating = false;
            }

            // wave
            if (m_isDart) {
                setYVelocity(m_playerSpeed * m_speedMultiplier * sign * (m_jumpBuffered ? 1 : -1),
                             0);

                if (m_gameLayer && m_wasJumpBuffered != m_jumpBuffered) {
                    m_gameLayer->gameEventTriggered(
                        m_jumpBuffered ? GJGameEvent::WavePush : GJGameEvent::WaveRelease, 0,
                        m_uniqueID);
                }
            }

            // swing
            else if (m_isSwing) {
                if (m_stateRingJump && m_jumpBuffered) {
                    m_stateRingJump = false;
                    double oldVel = m_yVelocity;
                    flipGravity(!m_isUpsideDown, true);
                    sign = flipMod();
                    setYVelocity(oldVel * .8, 0);

                    if (m_gameLayer) {
                        m_gameLayer->gameEventTriggered(GJGameEvent::SwingSwitch, 0, m_uniqueID);
                    }
                }

                // CHANGED - swing velocity
                // const float swingAccel = m_vehicleSize == 1.f ? .4f : .6f;
                const float swingAccel = .35f;
                setYVelocity(m_yVelocity - sign * dt * gravity * swingAccel, 0);
            }

            // ufo
            else if (m_isBird) {
                if (m_stateRingJump && m_jumpBuffered) {
                    m_stateRingJump = false;

                    // CHANGED - ufo jump velocity
                    // const double targetVel = (m_vehicleSize == 1.f ? 7.f : 8.f) * flightSizeScale
                    // * sign;
                    const double targetVel = 6.5f * flightSizeScale * sign;

                    // multiplying by sign is getting confusing
                    if (m_yVelocity * sign < targetVel * sign) {
                        setYVelocity(targetVel, 0);

                        // slope bonus velocity
                        if ((m_wasOnSlope || m_isOnSlope) && m_slopeVelocity > 0.f) {
                            double oldVel = m_yVelocity;
                            setYVelocity(m_yVelocity + m_slopeVelocity * .5f, 0);

                            double cap = oldVel * 1.4f;
                            if (m_yVelocity > cap) {
                                setYVelocity(cap, 0);
                            }
                        }

                        // play click particles with a delay
                        if (!levelFlipping() && !m_isHidden) {
                            m_ufoClickParticles->resumeSystem();
                            stopActionByTag(7);
                            auto seq = CCSequence::createWithTwoActions(
                                CCDelayTime::create(.12f),
                                CCCallFunc::create(
                                    this, callfunc_selector(PlayerObject::stopBurstEffect)));
                            seq->setTag(7);
                            runAction(seq);
                        }

                        if (m_gameLayer) {
                            m_gameLayer->gameEventTriggered(GJGameEvent::UFOJump, 0, m_uniqueID);
                        }
                    }
                }

                // gravity
                setYVelocity(m_yVelocity - sign * dt * gravity *
                                               (playerIsFallingBugged() ? 1.f : 1.2f) * .5f /
                                               flightSizeScale,
                             0);
            }

            // ship
            else {
                float gravityDir = 1.f;
                if (m_jumpBuffered) {
                    if (!m_isAccelerating || m_yVelocity * sign < 0) gravityDir = -1.f;
                } else if (!playerIsFallingBugged()) {
                    gravityDir = 1.2f;
                }

                float accelMult = m_jumpBuffered && playerIsFallingBugged() ? .5f : .4f;
                float platformerScale = m_isPlatformer ? .8f : 1.f;
                float effectiveGravity = gravity * platformerScale;

                setYVelocity(m_yVelocity - sign * dt * effectiveGravity * gravityDir * accelMult /
                                               flightSizeScale,
                             0);

                if (m_gameLayer && m_wasJumpBuffered != m_jumpBuffered) {
                    m_gameLayer->gameEventTriggered(
                        m_jumpBuffered ? GJGameEvent::ShipBoostStart : GJGameEvent::ShipBoostEnd, 0,
                        m_uniqueID);
                }
            }

            // velocity clamp
            if (!m_isAccelerating && !m_isDart) {
                double cap = 8.0 / flightSizeScale;
                if (!m_isUpsideDown) {
                    setYVelocity(std::clamp(m_yVelocity, -.8 * cap, cap), 0);
                } else {
                    setYVelocity(std::clamp(m_yVelocity, -cap, .8 * cap), 0);
                }
            }

            // ground particle related
            if (m_jumpBuffered) {
                m_isOnGround2 = false;
            }

            if (playerIsFallingBugged()) {
                m_maybeIsBoosted = false;
            }

            updateJumpFinalize();
            return;
        }

        // NOT FLYING

        float gravityMod;
        if (m_isBall || m_isSpider) {
            gravityMod = .6f;
        } else if (m_isRobot) {
            gravityMod = .9f;
        } else {
            gravityMod = 1.f;
        }

        bool isMovingLeft = m_platformerMovingLeft;
        bool isMovingRight = m_platformerMovingRight;
        bool inputMatchesMovement =
            (!isMovingLeft || holdingLeft) && (!isMovingRight || holdingRight);
        bool isCurrentlySliding = !inputMatchesMovement;

        bool canJumpFromSlide = true;
        if (isCurrentlySliding) {
            if (m_isSlidingRight != isMovingLeft && !m_slopeSlidingMaybeRotated) {
                m_maybeSlidingTime = 0;
            }

            canJumpFromSlide = m_maybeSlidingTime >= 2;
        }

        // jumping!!!!
        if (m_isOnGround && m_jumpBuffered && (!m_isRobot || m_stateRingJump)) {
            if (m_isSpider && !m_isDashing) {
                spiderTestJump(isCurrentlySliding);
                updateJumpFinalize();
                return;
            }

            // cube/ball/robot jump
            if (!m_isDashing && (!m_isPlatformer || canJumpFromSlide)) {
                m_maybeIsBoosted = true;
                m_isOnGround2 = false;
                m_isOnGround = false;
                m_stateRingJump = false;
                m_touchedPad = false;
                m_accelerationOrSpeed = 0.f;

                if (!isMovingLeft && !isMovingRight && m_maybeSlidingTime > 0) {
                    m_maybeSlidingTime = 0;
                    m_maybeSlidingStartTime = m_totalTime;
                }

                float jumpVelocity = m_yStart * (m_isRobot ? .5f : 1.f);

                // reduce jump height when sliding
                if (m_isPlatformer && isCurrentlySliding) {
                    if (!m_isMoving) {
                        if (m_maybeSlidingTime > 0) {
                            jumpVelocity *= 1.f - m_maybeSlidingTime * .3f;
                        }
                    } else {
                        double xSpeed = abs(m_platformerXVelocity);
                        double maxSpeed = m_playerSpeed * m_speedMultiplier;
                        if (xSpeed > maxSpeed) {
                            jumpVelocity = xSpeed / maxSpeed + jumpVelocity;
                        } else if (m_maybeSlidingTime > 0) {
                            jumpVelocity *= 1.f - m_maybeSlidingTime * .3f;
                        }
                    }

                    m_maybeSlidingTime++;
                }

                setYVelocity(sign * jumpVelocity * sizeScale, 0);

                // slope bonus velocity
                if (m_wasOnSlope || m_isOnSlope) {
                    float sv = m_slopeVelocity;
                    bool slopeBoostAllowed =
                        sign * sv > 0.f && (!m_isPlatformer || abs(m_platformerXVelocity) > 4.f);

                    if (slopeBoostAllowed) {
                        double timeOnSlope = m_totalTime - m_slopeStartTime;
                        double rampFactor = 1.0;
                        if (timeOnSlope < .1) {
                            rampFactor = std::max(timeOnSlope * 10.0, .4);
                        }

                        float slopeBoost = sv * (m_isBall ? 1 : rampFactor);
                        double oldVel = m_yVelocity;
                        setYVelocity(m_yVelocity + slopeBoost * .25, 0);

                        double cap = oldVel * 1.4f;
                        double vel = m_yVelocity;
                        if (!m_isUpsideDown) {
                            vel = std::min(vel, cap);
                        } else {
                            vel = std::max(vel, cap);
                        }
                        setYVelocity(vel, 0);
                    }
                }

                incrementJumps();

                if (m_isBall) {
                    flipGravity(!m_isUpsideDown, true);
                    // sign is no longer used
                    m_yVelocity *= .6f;
                    m_isOnGround3 = false;
                    m_jumpBuffered = false;
                } else if (m_isRobot) {
                    m_robotSprite->tweenToAnimation("jump_loop", .1f);
                } else {
                    m_isRotating = false;
                    m_isBallRotating2 = false;
                    m_isBallRotating = false;
                    m_rotationSpeed = 0.f;
                    runNormalRotation(false, 1.f);
                }

                if (m_gameLayer) {
                    GJGameEvent evt;
                    if (m_isBall) {
                        evt = GJGameEvent::BallSwitch;
                    } else if (m_isRobot) {
                        evt = GJGameEvent::RobotBoostStart;
                    } else if (m_isSpider) {
                        evt = GJGameEvent::SpiderTeleport;
                    } else {
                        evt = GJGameEvent::NormalJump;
                    }

                    m_gameLayer->gameEventTriggered(evt, 0, m_uniqueID);
                }

                if (m_isPlatformer) {
                    animatePlatformerJump(1.f);
                }

                updateJumpFinalize();
                return;
            }
        }

        // robot jump
        if (m_maybeIsBoosted) {
            if (m_isRobot && m_jumpBuffered && !m_touchedPad && m_accelerationOrSpeed < 1.5) {
                m_accelerationOrSpeed += dt * .1f;
                setYVelocity(m_yVelocity + sign * dt * gravity * gravityMod, 0);
            }

            setYVelocity(m_yVelocity - sign * dt * gravity * gravityMod, 0);

            if (m_gameLayer && m_isRobot && m_touchedPad && !m_wasRobotJump) {
                m_gameLayer->gameEventTriggered(GJGameEvent::RobotBoostStop, 0, m_uniqueID);
            }

            if (!playerIsFallingBugged() && !(m_isPlatformer && m_yVelocity * sign <= 0)) {
                updateJumpFinalize();
                return;
            }

            m_maybeIsBoosted = false;
            m_maybeIsFalling = true;
            m_isOnGround2 = false;
            m_fallStartY = getPositionY();

            // falling animation
            if (m_isRobot) {
                m_robotSprite->tweenToAnimation("fall_loop", .1f);
                if (m_gameLayer && !m_touchedPad) {
                    m_gameLayer->gameEventTriggered(GJGameEvent::RobotBoostStop, 0, m_uniqueID);
                }
            } else if (m_isSpider) {
                m_spiderSprite->tweenToAnimation("fall_loop", .1f);
            }

            updateJumpFinalize();
            return;
        }

        if (m_isOnGround) {
            m_fallStartY = getPositionY();
        }

        // coyote time
        if (playerIsFallingBugged() &&
            (!m_isPlatformer || m_totalTime - m_lastLandTime >= .05f || !canJumpFromSlide)) {
            m_isOnGround = false;
        }

        // gravity
        setYVelocity(m_yVelocity - sign * dt * gravity * gravityMod, 0);

        // velocity clamp
        if (!m_isUpsideDown) {
            setYVelocity(std::max(m_yVelocity, -15.0), 0);
        } else {
            setYVelocity(std::min(m_yVelocity, 15.0), 0);
        }

        // maybe a replacement for setYVelocity above
        // if (vel != (int)vel)
        // {
        //     double fractional = round(vel * 1000.0) / 1000.0;
        //     vel = (int)vel + fractional - (int)fractional;
        // }

        // reset rotation when falling
        if (!(m_yVelocity * sign >= -.25 && m_yVelocity * sign <= 0) && !m_isBall && !m_isSpider &&
            !m_isRobot && !m_isRotating && !m_isOnSlope && !m_isCollidingWithSlope &&
            !(m_isPlatformer && m_isOnGround2) && !m_isLocked && !m_isDashing) {
            m_isRotating = false;
            m_isBallRotating2 = false;
            m_isBallRotating = false;
            m_rotationSpeed = 0.f;
            runNormalRotation(false, 1.f);
        }

        // falling animation
        if (playerIsFallingBugged() && m_yVelocity * sign <= -4.f) {
            if (!m_isOnGround2 && !m_maybeSpriteRelated) {
                if (m_isRobot) {
                    m_robotSprite->tweenToAnimation("fall_loop", .1f);
                } else if (m_isSpider) {
                    m_spiderSprite->tweenToAnimation("fall_loop", .1f);
                }
            }

            m_isOnGround2 = false;
        }

        updateJumpFinalize();
    }

    void updateJumpFinalize() {
        m_wasJumpBuffered = m_jumpBuffered;
        m_wasRobotJump = m_touchedPad;
    }
};
