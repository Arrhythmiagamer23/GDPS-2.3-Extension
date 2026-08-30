#pragma once

#include <smjs.object-collab/include/object_collab.hpp>
#include "../ui/CloneStyleSetting.hpp"

class StyleNode;
class CosmicClonesController;

class CosmicClonesTrigger : public object_collab::CustomObject<EffectGameObject> {
public:
    static CosmicClonesTrigger* create(object_collab::ObjectInfo* info);
    static object_collab::PopupOptions getEditObjectConfig(const object_collab::Selected& selected);

    std::shared_ptr<CosmicClonesController> m_controller = nullptr;

    int m_count = 3;
    float m_initialDelay = 1.75f;
    float m_delay = 1;
    bool m_disabled = false;
    int m_controllerId = 1;
    bool m_damage = true;
    bool m_stopper = false;
    bool m_sfx = true;
    std::vector<Style> m_styles = {{"Cosmic Mario\n(SMG 1)"}};

    CosmicClonesTrigger(object_collab::ObjectInfo* info);
    void postInit() override;
    void triggerObject(GJBaseGameLayer* layer, const int uniqueID, const gd::vector<int>* remapKeys) override;
    std::vector<std::string> getObjectDetails() override;
};

class StylePickMenu : public cocos2d::CCMenu {
public:
    void init(const object_collab::Selected& selected);
    static StylePickMenu* create(const object_collab::Selected& selected);

    void updateState();

    geode::ScrollLayer* m_styleScroll = nullptr;
    geode::Button* m_newBtn = nullptr;
    std::vector<StyleNode*> m_nodes;
    std::vector<CosmicClonesTrigger*> m_selected;
};


class StyleNode : public cocos2d::CCMenu {
protected:
    bool init(StylePickMenu* menu, Style value);
;
    Style m_style;
    cocos2d::CCLabelBMFont* m_label = nullptr;
    StylePickMenu* m_menu = nullptr;
    CCMenu* m_customMenu = nullptr;

    SimplePlayer* m_preview = nullptr;
public:
    void updateState(const Style& style);
    const Style& getStyle() { return m_style; }

    StylePickMenu* getMenu() const;

    static StyleNode* create(StylePickMenu* menu, const Style& value);
};
