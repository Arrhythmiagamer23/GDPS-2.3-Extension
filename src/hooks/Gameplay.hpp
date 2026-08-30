#pragma once

#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

#ifdef GEODE_IS_MACOS
#define tickHook() processQueuedButtons(float dt, bool clear)
#define tickOrig() processQueuedButtons(dt, clear)
#else
#define tickHook() processCommands(float dt, bool half, bool last)
#define tickOrig() processCommands(dt, half, last)
#endif

struct OldClonesTrigger;
class CosmicClonesController;

// Base Gameplay

class $modify(CosmicClonesGJBGL, GJBaseGameLayer)
{
    struct Fields {
        unsigned int m_p1Immunity = 0;
        unsigned int m_p2Immunity = 0;

        bool m_p1Jump = false;
        bool m_p2Jump = false;
        bool m_p1Frozen = false;
        bool m_p2Frozen = false;

		bool m_autoClones = false;
		std::shared_ptr<CosmicClonesController> m_controller;
        std::unordered_map<int, std::shared_ptr<CosmicClonesController>> m_triggerControllers;
    };

    void tickHook();
};

class $modify(CosmicClonesPlayerObject, PlayerObject) {
    void incrementJumps();
    void enablePlayerControls();
    void disablePlayerControls();
};


// PlayLayer Gameplay

class $modify(CosmicClonesPlayLayer, PlayLayer) {
    void resetLevel();
    void fullReset();
    void setupHasCompleted();
    void levelComplete();
};


// Editor Gameplay

class $modify(CosmicClonesLevelEditorLayer, LevelEditorLayer) {
    bool init(GJGameLevel* level, bool noUI);
    void onPlaytest();
    void onStopPlaytest();
};