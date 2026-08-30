#include "Gameplay.hpp"

#include <Geode/Geode.hpp>

#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <ninxout.options_api/include/API.hpp>

#include "../Utils.hpp"
#include "../internal/CosmicClonesController.hpp"

using namespace geode::prelude;

// Base Gameplay

void CosmicClonesGJBGL::tickHook() {
    auto fields = m_fields.self();

    fields->m_controller->tick(m_gameState.m_currentProgress);
    for (auto [id, cont] : fields->m_triggerControllers) {
        cont->tick(m_gameState.m_currentProgress);
    }

    GJBaseGameLayer::tickOrig();
}

void CosmicClonesPlayerObject::incrementJumps() {
    PlayerObject::incrementJumps();
    if (!m_gameLayer) return;
    auto fields = reinterpret_cast<CosmicClonesGJBGL*>(m_gameLayer)->m_fields.self();
    if (this == m_gameLayer->m_player1) {
        fields->m_p1Jump = true;
    } else if (this == m_gameLayer->m_player2) {
        fields->m_p2Jump = true;
    }
}

void CosmicClonesPlayerObject::enablePlayerControls() {
    PlayerObject::enablePlayerControls();
    if (!m_gameLayer) return;
    auto fields = reinterpret_cast<CosmicClonesGJBGL*>(m_gameLayer)->m_fields.self();
    if (this == m_gameLayer->m_player1) {
        fields->m_p1Frozen = false;
        fields->m_p1Immunity = 240;
    } else if (this == m_gameLayer->m_player2) {
        fields->m_p2Frozen = false;
        fields->m_p2Immunity = 240;
    }
}

void CosmicClonesPlayerObject::disablePlayerControls() {
    PlayerObject::disablePlayerControls();
    if (!m_gameLayer) return;
    auto fields = reinterpret_cast<CosmicClonesGJBGL*>(m_gameLayer)->m_fields.self();
    if (this == m_gameLayer->m_player1) {
        fields->m_p1Frozen = true;
    } else if (this == m_gameLayer->m_player2) {
        fields->m_p2Frozen = true;
    }
}


// PlayLayer Gameplay

inline bool isEnabledForLevel(GJGameLevel* level) {
    if (level->m_levelType == GJLevelType::Editor) {
        return Mod::get()->getSavedValue<bool>(fmt::format("enable-in-editor-{}", EditorIDs::getID(level)));
    }
    return Mod::get()->getSavedValue<bool>(fmt::format("enable-in-{}", level->m_levelID));
}

void CosmicClonesPlayLayer::resetLevel() {
    PlayLayer::resetLevel();
    auto bgl = reinterpret_cast<CosmicClonesGJBGL*>(this);
    auto fields = bgl->m_fields.self();
    if (!fields->m_controller) return;
    fields->m_autoClones = getSettingFast<"enabled", bool>() || isEnabledForLevel(m_level);

    // wow one of my first actual explaining comments
    // basically in practice mode, it'd be better if the clones reset to where they were at said checkpoint
    // so we don't reset them.
    // we also give the player a second of immunity in case of spawn trap
    if (m_isPracticeMode) {
        fields->m_p1Immunity = 240; // 1s immunity (except for stinky tps bypass)
        fields->m_p2Immunity = 240;

        if (fields->m_autoClones && !fields->m_controller->isStopped()) fields->m_controller->softReset(m_gameState.m_currentProgress);
    } else {
        if (!fields->m_controller->isStopped()) fields->m_controller->stop(true);
        for (const auto& [id, cont] : fields->m_triggerControllers){
            if (!cont->isStopped()) {
                cont->stop(true);
                if (cont->getOffset() <= m_gameState.m_currentProgress) cont->start();
            }
        }

        fields->m_controller->loadConfigFromSettings();
        if (fields->m_autoClones) fields->m_controller->start();
    }
}

void CosmicClonesPlayLayer::fullReset() {
    PlayLayer::fullReset();
    auto bgl = reinterpret_cast<CosmicClonesGJBGL*>(this);
    auto fields = bgl->m_fields.self();
    if (!fields->m_controller->isStopped()) fields->m_controller->stop(true);
    for (const auto& [id, cont] : fields->m_triggerControllers){
        if (!cont->isStopped()) cont->stop(true);
    }

    fields->m_autoClones = getSettingFast<"enabled", bool>() || isEnabledForLevel(m_level);
    fields->m_controller->loadConfigFromSettings();
    if (fields->m_autoClones) fields->m_controller->start();
}

void CosmicClonesPlayLayer::setupHasCompleted() {
    PlayLayer::setupHasCompleted();
    auto bgl = reinterpret_cast<CosmicClonesGJBGL*>(this);
    auto fields = bgl->m_fields.self();
    fields->m_autoClones = getSettingFast<"enabled", bool>() || isEnabledForLevel(m_level);
    fields->m_controller = CosmicClonesController::createWithSettings(bgl);
    if (fields->m_autoClones) fields->m_controller->start();
}

void CosmicClonesPlayLayer::levelComplete() {
    PlayLayer::levelComplete();
    auto fields = reinterpret_cast<CosmicClonesGJBGL*>(this)->m_fields.self();
    if (!fields->m_controller->isStopped()) fields->m_controller->stop();
    for (const auto& [id, cont] : fields->m_triggerControllers){
        if (!cont->isStopped()) cont->stop();
    }
}

// Editor Gameplay

bool CosmicClonesLevelEditorLayer::init(GJGameLevel* level, bool noUI) {
    if (!LevelEditorLayer::init(level, noUI)) return false;
    auto bgl = reinterpret_cast<CosmicClonesGJBGL*>(this);
    auto fields = bgl->m_fields.self();
    fields->m_autoClones = getSettingFast<"enabled", bool>();
    fields->m_controller = CosmicClonesController::createWithSettings(bgl);
    return true;
}

void CosmicClonesLevelEditorLayer::onPlaytest() {
    auto bgl = reinterpret_cast<CosmicClonesGJBGL*>(this);
    auto fields = bgl->m_fields.self();
    LevelEditorLayer::onPlaytest();
    fields->m_autoClones = getSettingFast<"enabled", bool>() && getSettingFast<"editor", bool>();
    if (!fields->m_autoClones) return;
    fields->m_controller->loadConfigFromSettings();
    fields->m_controller->start();
    for (const auto& [id, cont] : fields->m_triggerControllers){
        cont->loadConfigFromTrigger();
    }
}

void CosmicClonesLevelEditorLayer::onStopPlaytest() {
    auto bgl = reinterpret_cast<CosmicClonesGJBGL*>(this);
    auto fields = bgl->m_fields.self();
    LevelEditorLayer::onStopPlaytest();
    if (!fields->m_controller->isStopped()) fields->m_controller->stop(true);
    for (const auto& [id, cont] : fields->m_triggerControllers){
        if (!cont->isStopped()) cont->stop(true);
    }
}

// Misc

$on_mod(Loaded) {
    OptionsAPI::addPreLevelSetting<bool>(
        "Clones in This Level",
        "clones-in-level"_spr,
        [](GJGameLevel* level) {
            std::string key;
            if (level->m_levelType != GJLevelType::Editor) {
                key = fmt::format("enable-in-{}", level->m_levelID);
            } else {
                key = fmt::format("enable-in-editor-{}", EditorIDs::getID(level));
            }
            Mod::get()->setSavedValue<bool>(key, !Mod::get()->getSavedValue<bool>(key, false));
        },
        [](GJGameLevel* level) {
            return isEnabledForLevel(level);
        },
        "If clones should spawn from the start in this level. No effect if \"Clones in Every Level\" is enabled."
    );
}
