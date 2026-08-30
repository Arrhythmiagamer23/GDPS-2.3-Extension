#include "Editor.hpp"

#include <Geode/Geode.hpp>

#include <smjs.object-collab/include/ObjectAPI.hpp>

#include "../internal/CosmicClonesTrigger.hpp"
#include "Gameplay.hpp"
#include "../Utils.hpp"
#include "../ui/CloneStyleSetting.hpp"

// this is a very specific reference
#define CLONE_ID 11012007

using namespace geode::prelude;

void OldClonesTrigger::customObjectSetup(gd::vector<gd::string>& values, gd::vector<void*>& exists) {
    SequenceTriggerGameObject::customObjectSetup(values, exists);
    if (m_sequenceMode != CLONE_ID) return;
    queueInMainThread([self = Ref(this)] {
        auto lel = LevelEditorLayer::get();
        if (!lel && !GJBaseGameLayer::get()) return;
        auto id = object_collab::ObjectAPI::getCustomObjectNumericID("cosmic-clones-trigger"_spr);
        if (!id.has_value()) {
            log::error("Something went wrong, cannot find numeric id for the cosmic clones trigger!");
            return;
        }
        CosmicClonesTrigger* obj;
        if (lel) obj = static_cast<CosmicClonesTrigger*>(lel->createObject(id.value(), self->m_obPosition, true));
        else obj = static_cast<CosmicClonesTrigger*>(createWithKey(id.value()));
        obj->m_count = self->getCount();
        obj->m_initialDelay = self->getStartDelay();
        obj->m_delay = self->getDelay();
        obj->m_disabled = self->isDisabled();
        obj->m_controllerId = self->getControllerID();
        obj->m_damage = self->isDamaging();
        obj->m_stopper = self->isStopper();
        obj->m_sfx = self->isSfx();
        obj->m_styles = self->getStyles();
        obj->m_isTouchTriggered = self->m_isTouchTriggered;
        obj->m_isSpawnTriggered = self->m_isSpawnTriggered;
        obj->m_isMultiTriggered = self->m_isMultiTriggered;
        obj->m_groups = self->m_groups;
        obj->m_channelValue = self->m_channelValue;
        obj->m_ordValue = self->m_ordValue;
        obj->m_editorLayer = self->m_editorLayer;
        obj->m_editorLayer2 = self->m_editorLayer2;
        obj->m_zLayer = self->m_zLayer;
        obj->m_zOrder = self->m_zOrder;
        obj->m_isNoTouch = self->m_isNoTouch;
        if (lel) {
            lel->removeObject(self, true);
        } else {
            self->m_fields->realTrigger = obj;
            log::dev("clone trigger forward set up");
        }
    });
}

void OldClonesTrigger::triggerObject(GJBaseGameLayer* layer, int uniqueID, gd::vector<int> const* remapKeys) {
    if (auto real = m_fields->realTrigger) {
        real->triggerObject(layer, uniqueID, remapKeys);
        return;
    }
    SequenceTriggerGameObject::triggerObject(layer, uniqueID, remapKeys);
}

int OldClonesTrigger::getCount() const {
    return -m_resetMode;
}

float OldClonesTrigger::getStartDelay() const {
    return -m_reset;
}

float OldClonesTrigger::getDelay() const {
    return -m_minInt;
}

bool OldClonesTrigger::isDamaging() const {
    if (m_chanceObjects.size() < 1) return true;
    return (m_chanceObjects[0].m_groupID & 1 << 0) != 0;
}

bool OldClonesTrigger::isStopper() const {
    if (m_chanceObjects.size() < 1) return false;
    return m_chanceObjects[0].m_groupID & 1 << 1;
}

bool OldClonesTrigger::isSfx() const {
    if (m_chanceObjects.size() < 1) return false;
    return m_chanceObjects[0].m_groupID & 1 << 2;
}

bool OldClonesTrigger::isDisabled() const {
    return m_uniqueRemap;
}

int OldClonesTrigger::getControllerID() const {
    return -m_chanceObjects[0].m_chance;
}

static ccColor3B colorFromInt(int i) {
    return {
        .r = static_cast<GLubyte>(i >> 16 & 0xFF),
        .g = static_cast<GLubyte>(i >> 8 & 0xFF),
        .b = static_cast<GLubyte>(i & 0xFF)
    };
}

std::vector<Style> OldClonesTrigger::getStyles() const {
    if (10 >= m_chanceObjects.size()) return {};
    std::vector<Style> ret;
    for (int i = 10; i < m_chanceObjects.size();) {
        auto co = m_chanceObjects[i];
        if (co.m_groupID == -1) ret.push_back({"Cosmic Mario\n(SMG 1)"});
        else if (co.m_groupID == -2) ret.push_back({"Cosmic Clone\n(SMG 2)"});
        else if (co.m_groupID == -3) ret.push_back({"Badeline Chaser\n(Celeste)"});
        else if (co.m_groupID == -4) {
            ret.push_back(chanceObjectsToCustomStyle(co, i + 1 == m_chanceObjects.size() ? ChanceObject{} : m_chanceObjects[i  + 1]));
            i++;
        }
        else if (co.m_groupID == -5) ret.push_back({"Hungry Luma"});
        else if (co.m_groupID == -6) ret.push_back({"The Yellow One"});
        else if (co.m_groupID == -7) ret.push_back({"eri"});
        i++;
    }
    if (ret.empty()) return getSettingFast<"styles", std::vector<Style>>();
    return ret;
}

Style OldClonesTrigger::chanceObjectsToCustomStyle(const ChanceObject& first, const ChanceObject& second) {
    auto col1 = colorFromInt(-first.m_chance);
    auto col2 = colorFromInt(-second.m_groupID);
    bool glowOn = -second.m_chance >> 24 & 1;
    auto glow = colorFromInt(-second.m_chance);
    return {"Custom", col1, col2, glowOn, glow};
}

// this was filler in case i added more data
void OldClonesTrigger::fill10IfNeeded() {
    if (m_chanceObjects.size() < 10) {
        for (int i = 0; m_chanceObjects.size() < 10; i++) {
            m_chanceObjects.emplace_back(-1, -1);
        }
    }
}