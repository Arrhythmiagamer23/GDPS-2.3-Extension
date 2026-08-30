#pragma once

#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/SequenceTriggerGameObject.hpp>

#include "../internal/CosmicClonesTrigger.hpp"

struct Style;

// Property mappings
// int   count:        int   m_resetMode
// float startDelay:   float m_reset
// float delay:        float m_minInt
// bool  isDisabled:   bool  m_uniqueRemap
// int   controllerId: int   m_chanceObjects[0].m_chance
// these ones use bitwise operators
// bool  damaging:     int   m_chanceObjects[0].m_groupID (bit 0)
// bool  stopper:      int   m_chanceObjects[0].m_groupID (bit 1)
// bool  sfx:          int   m_chanceObjects[0].m_groupID (bit 2)

// Styles are all chance objects after index 10.
// Chance object mapping for styles
// m_groupID: type
// m_chance: (custom) style color 1
// Second object (when type == "Custom")
// m_groupID: (custom) style color 2
// m_chance: (custom) glow

class $modify(OldClonesTrigger, SequenceTriggerGameObject) {

    struct Fields {
        geode::Ref<CosmicClonesTrigger> realTrigger = nullptr;
    };

    void customObjectSetup(gd::vector<gd::string>& values, gd::vector<void*>& exists);
    void triggerObject(GJBaseGameLayer* layer, int uniqueID, gd::vector<int> const* remapKeys);

    int getCount() const;
    float getStartDelay() const;
    float getDelay() const;
    bool isDamaging() const;
    bool isDisabled() const;
    int getControllerID() const;
    bool isStopper() const;
    bool isSfx() const;
    std::vector<Style> getStyles() const;

    void fill10IfNeeded();

    static Style chanceObjectsToCustomStyle(const ChanceObject& first, const ChanceObject& second);
};