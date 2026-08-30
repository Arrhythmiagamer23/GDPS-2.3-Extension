#include "CosmicClonesTrigger.hpp"

#include "../hooks/Gameplay.hpp"
#include "ShaderManager.hpp"
#include "CosmicClonesController.hpp"
#include "../ui/CloneStyleSetting.hpp"
#include "../Utils.hpp"
#include <vector>

using namespace object_collab::prelude;
using namespace geode::prelude;

CosmicClonesTrigger* CosmicClonesTrigger::create(ObjectInfo* info) {
    auto ret = new CosmicClonesTrigger(info);
    queueInMainThread([ret] {
        auto inner = CCSprite::createWithSpriteFrameName("triggerInner.png"_spr);
        inner->setID("inner"_spr);
        inner->setShaderProgram(ShaderManager::get().getCosmicShader());
        ret->addChildAtPosition(inner, Anchor::Center, {0, 0}, false);
    });
    return ret;
}

PopupOptions CosmicClonesTrigger::getEditObjectConfig(const Selected& selected) {
    std::vector<std::unique_ptr<ToggleMenu>> toggles;

    toggles.push_back(ToggleMenu::builder()
            .title("SFX")
                .id("sfx"_spr)
                .onValue([](const bool value, const Selected& selected, Popup* popup) {
                    applyValueToSelected(selected, &CosmicClonesTrigger::m_sfx, value);
                })
                .currentValue([](const Selected& selected, Popup* popup) {
                    return getCommonValueOrDefault(selected, &CosmicClonesTrigger::m_sfx);
                })
                .build());
    toggles.push_back(ToggleMenu::builder()
            .title("Stop")
            .id("stop"_spr)
            #define hide(id) if (auto child = popup->getChildByIDRecursive(id##_spr)) child->setVisible(!value); else log::error("Could not find {}!", id##_spr)
            #define hideAll() \
                hide("clone-count"); \
                hide("delay"); \
                hide("start-delay"); \
                hide("sfx"); \
                hide("damage"); \
                hide("styles")
            .onValue([](const bool value, const Selected& selected, Popup* popup) {
                applyValueToSelected(selected, &CosmicClonesTrigger::m_stopper, value);
                hideAll();
            })
            .currentValue([](const Selected& selected, Popup* popup) {
                auto value = getCommonValueOrDefault(selected, &CosmicClonesTrigger::m_stopper);
                popup->runAction(CallFuncExt::create([popup, value  ] {
                    hideAll();
                }));
                return value;
            #undef hide
            })
            .build());
    toggles.push_back(ToggleMenu::builder()
            .title("Damage")
            .id("damage"_spr)
            .onValue([](const bool value, const Selected& selected, Popup* popup) {
                applyValueToSelected(selected, &CosmicClonesTrigger::m_damage, value);
            })
            .currentValue([](const Selected& selected, Popup* popup) {
                return getCommonValueOrDefault(selected, &CosmicClonesTrigger::m_damage);
            })
            .build());
    toggles.push_back(ToggleMenu::builder()
            .title("Disable")
            .id("disable"_spr)
            .onValue([](const bool value, const Selected& selected, Popup* popup) {
                applyValueToSelected(selected, &CosmicClonesTrigger::m_disabled, value);
            })
            .currentValue([](const Selected& selected, Popup* popup) {
                return getCommonValueOrDefault(selected, &CosmicClonesTrigger::m_disabled);
            })
            .build());

    return PopupConfig::builder()
        .width(320)
        .height(250)
        .gapY(20)
        .title("Cosmic Clones Trigger")
        .info(InfoPopup::builder()
            .title("Clones Trigger Help")
            .description("Spawns <cg>cosmic clones</c> that chase the player.\n"
            "<cy>Clone count</c> changes how many clones are spawned.\n"
            "<cb>Controller ID</c> manages what clone controller the trigger manages. A clone controller can only have one set of clones at a time.\n"
            "<cr>Stop</c> makes the controller kill the clones when triggered.\n"
            "<cd>Start delay</c> changes how far behind the player the first clone is.\n"
            "<co>Delay</c> changes the distance between clones.\n"
            "<cf>Damage</c> controls if the clones can kill you.\n"
            "<cs>Styles</c> change how the clones look.\n"
            "<cj>Disable</c> disables the trigger.")
            .build())
        .triggerToggles(true)
        .menu(AxisLayoutMenu::builder()
            .axis(Axis::Row)
            .gap(20)
            .crossAxisAlignment(CrossAxisAlignment::Center)
            .menu(AxisLayoutMenu::builder()
                .axis(Axis::Column)
                .gap(20)
                .mainAxisDirection(AxisDirection::TopToBottom)
                .crossAxisAlignment(CrossAxisAlignment::Center)
                .menu(NumericMenu::builder()
                    .id("clone-count"_spr)
                    .title("Clone Count")
                    .inputType(NumericMenu::InputType::Arrows)
                    .min(1)
                    .max(20)
                    .precision(0)
                    .onValue([](const int value, const Selected& selected, Popup* popup) {
                        applyValueToSelected(selected, &CosmicClonesTrigger::m_count, std::clamp(value, 1, 20));
                    })
                    .currentValue([](const Selected& selected, Popup* popup) {
                        return getCommonValueOrDefault<int>(selected, &CosmicClonesTrigger::m_count);
                    })
                    .build())
                .menu(NumericMenu::builder()
                    .id("controller-id"_spr)
                    .title("Controller Id")
                    .inputType(NumericMenu::InputType::Arrows)
                    .min(1)
                    .max(100)
                    .precision(0)
                    .onValue([](const int value, const Selected& selected, Popup* popup) {
                        applyValueToSelected(selected, &CosmicClonesTrigger::m_controllerId, std::clamp(value, 1, 100));
                    })
                    .currentValue([](const Selected& selected, Popup* popup) {
                        return getCommonValueOrDefault<int>(selected, &CosmicClonesTrigger::m_controllerId);
                    })
                    .build())
                .menu(NumericMenu::builder()
                    .id("delay"_spr)
                    .title("Delay")
                    .inputType(NumericMenu::InputType::Slider)
                    .min(0.25)
                    .max(20)
                    .stepSize(0.25)
                    .onValue([](const float value, const Selected& selected, Popup* popup) {
                        applyValueToSelected(selected, &CosmicClonesTrigger::m_delay, value);
                    })
                    .currentValue([](const Selected& selected, Popup* popup) {
                        return getCommonValueOrDefault<float>(selected, &CosmicClonesTrigger::m_delay);
                    })
                    .build())
                .menu(NumericMenu::builder()
                    .id("start-delay"_spr)
                    .title("Start Delay")
                    .inputType(NumericMenu::InputType::Slider)
                    .min(0.25)
                    .max(20)
                    .stepSize(0.25)
                    .onValue([](const float value, const Selected& selected, Popup* popup) {
                        applyValueToSelected(selected, &CosmicClonesTrigger::m_initialDelay, value);
                    })
                    .currentValue([](const Selected& selected, Popup* popup) {
                        return getCommonValueOrDefault<float>(selected, &CosmicClonesTrigger::m_initialDelay);
                    })
                    .build())
                .build())
            .menu(CustomValueMenu::builder()
                .id("styles"_spr)
                .title("Clone Styles")
                .factory([](const Selected& selected, geode::Popup* popup) {
                    auto menu = StylePickMenu::create(selected);
                    // Don't say it. Not a word.
                    menu->setContentSize(menu->getContentSize() * 7.f / 6);
                    menu->runAction(CallFuncExt::create([menu]{
                        menu->setContentSize(menu->getContentSize() * 6.f / 7);
                    }));
                    menu->setScale(7.f / 6);
                    return menu;
                })
                .build())
            .build())
        .rightToggles(std::move(toggles))
        .build();
}

CosmicClonesTrigger::CosmicClonesTrigger(ObjectInfo* info): CustomObject(info, ObjectTraits::builder()
    .gameObjectType(GameObjectType::Modifier)
    .ignoreEditorDuration(true)
    .build()) { }

void CosmicClonesTrigger::postInit() {
    this->setHitbox({ 1, 1 });
}

void CosmicClonesTrigger::triggerObject(GJBaseGameLayer* layer, const int uniqueID, const gd::vector<int>* remapKeys) {
    log::dev("Clone Trigger:\n"
              "Controller ID: {}\n"
              "Count: {}\n"
              "Start Delay: {}\n"
              "Delay: {}\n"
              "Damage Enabled: {}\n"
              "Disabled: {}\n"
              "Stop: {}\n"
              "Styles: {}",
        m_controllerId,
        m_count,
        m_initialDelay,
        m_delay,
        m_damage,
        m_disabled,
        m_stopper,
        matjson::Value(m_styles).dump());
    if (!m_disabled) {
        auto bgl = static_cast<CosmicClonesGJBGL*>(layer);
        auto bglFields = bgl->m_fields.self();
        auto id = m_controllerId;
        auto it = bglFields->m_triggerControllers.find(id);
        if (m_stopper) {
            if (it == bglFields->m_triggerControllers.end() || it->second == nullptr) return;
            if (it->second->isStopped()) return;
            it->second->stop();
            return;
        }
        if (it == bglFields->m_triggerControllers.end() || it->second == nullptr) {
            auto controller = CosmicClonesController::createFromTrigger(bgl, this);
            bglFields->m_triggerControllers[id] = controller;
            controller->loadConfigFromTrigger(this);
            controller->start();
        } else {
            if (!it->second->isStopped()) return;
            it->second->loadConfigFromTrigger(this);
            it->second->start();
        }
    }
}

std::vector<std::string> CosmicClonesTrigger::getObjectDetails() {
    return DetailsBuilder::builder()
        .field("Controller ID: {}", m_controllerId)
        .field("Count: {}", m_targetGroupID)
        .field("Start Delay: {}", m_initialDelay)
        .field("Delay: {}", m_delay)
        .field("Damage Enabled: {}", m_damage)
        .field("Disabled: {}", m_disabled)
        .field("Styles: {}", matjson::Value(m_styles).dump())
        .field("Stop: {}", m_stopper)
        .build();
}

void StylePickMenu::updateState() {
    std::vector<Style> styles;
    for (auto node : m_nodes) {
        styles.push_back(node->getStyle());
    }
    for (auto trigger : m_selected) {
        trigger->m_styles = styles;
    }

    float height = m_styleScroll->m_contentLayer->getContentHeight();
    m_styleScroll->m_contentLayer->updateLayout();
    m_styleScroll->m_contentLayer->setPositionY(m_styleScroll->m_contentLayer->getPositionY() - (m_styleScroll->m_contentLayer->getContentHeight() - height));
    auto diff = -m_styleScroll->m_contentLayer->getPositionY() + m_styleScroll->getContentHeight() - m_styleScroll->m_contentLayer->getContentHeight();
    if (diff > 0) m_styleScroll->m_contentLayer->setPositionY(m_styleScroll->m_contentLayer->getPositionY() + diff);
    m_styleScroll->scrollWheel(0, 0);

    bool shouldEnable = m_styleScroll->m_contentLayer->getChildrenExt().size() - 3 < 20;
    m_newBtn->setEnabled(shouldEnable);
    m_newBtn->setOpacity(shouldEnable ? 255 : 127);
}

StylePickMenu* StylePickMenu::create(const Selected& selected) {
    auto ret = new StylePickMenu;
    ret->init(selected);
    ret->autorelease();
    return ret;
}

void StylePickMenu::init(const Selected& selected) {
    setAnchorPoint({0.5, 0.5});
    ignoreAnchorPointForPosition(false);
    setContentSize({169, 180});
    setID("style-pick-menu");

    auto scrollBg = NineSlice::create("square02_001.png");
    scrollBg->setID("background");
    scrollBg->setContentSize({169, 180});
    scrollBg->setOpacity(50);
    addChildAtPosition(scrollBg, Anchor::Center, {0, 10});

    CosmicClonesTrigger* first = nullptr;
    bool differ = false;
    for (auto coi : selected) {
        auto trig = typeinfo_cast<CosmicClonesTrigger*>(coi);
        if (!trig) return;
        m_selected.push_back(trig);
        if (first == nullptr) first = trig;
        else if (trig->m_styles != first->m_styles) differ = true;
    }
    if (first == nullptr) return; // this should never happen but just in case
    if (differ) {
        auto lab = CCLabelBMFont::create("Styles do not match.\nCannot multi-edit.", "bigFont.fnt");
        lab->setScale(0.45f);
        lab->setID("unmatch-label");
        addChildAtPosition(lab, Anchor::Center, {0, 10});
        return;
    }

    m_styleScroll = ScrollLayer::create({155, 180});
    m_styleScroll->ignoreAnchorPointForPosition(false);
    m_styleScroll->setZOrder(1);
    m_styleScroll->setID("style-scroll");
    addChildAtPosition(m_styleScroll, Anchor::Center, {0, 10});

    auto topSpacer = CCNode::create();
    topSpacer->setContentHeight(2);
    topSpacer->setID("top-spacer");
    m_styleScroll->m_contentLayer->addChild(topSpacer, 2);

    auto botSpacer = CCNode::create();
    botSpacer->setContentHeight(2);
    botSpacer->setID("bottom-spacer");
    m_styleScroll->m_contentLayer->addChild(botSpacer, -1);

    m_newBtn = Button::createWithSpriteFrameName("edit_addCBtn_001.png", [this](auto btn) {
        auto node = StyleNode::create(this, {});
        node->updateState({});
        m_styleScroll->m_contentLayer->addChild(node);
        m_nodes.push_back(node);
        updateState();
    });
    m_newBtn->setID("new-button");
    m_newBtn->setScale(.5f);
    m_newBtn->setContentSize({30, 30});
    m_styleScroll->m_contentLayer->addChild(m_newBtn, 1);

    for (const auto& style : first->m_styles) {
        auto node = StyleNode::create(this, style);
        node->updateState(style);
        m_nodes.push_back(node);
        m_styleScroll->m_contentLayer->addChild(node);
    }

    bool shouldEnable = m_styleScroll->m_contentLayer->getChildrenExt().size() - 3 < 20;
    m_newBtn->setEnabled(shouldEnable);
    m_newBtn->setOpacity(shouldEnable ? 255 : 127);

    m_styleScroll->m_contentLayer->setLayout(ColumnLayout::create()->setAxisReverse(true)->setAxisAlignment(AxisAlignment::End)->ignoreInvisibleChildren(false)->setAutoScale(false)->setCrossAxisOverflow(false)->setAutoGrowAxis(180));
    m_styleScroll->scrollToTop();
}

StylePickMenu* StyleNode::getMenu() const {
    return m_menu;
}

bool StyleNode::init(StylePickMenu* popup, Style value) {
    if (!CCMenu::init()) return false;
    setID("clone-style-node");
    m_menu = popup;
    m_style = std::move(value);
    ignoreAnchorPointForPosition(false);
    setAnchorPoint({.5f, .5f});
    setContentSize({155, m_style.type == "Custom" ? 120.f : 30.f});
    m_label = CCLabelBMFont::create(value.type.c_str(), "bigFont.fnt");
    m_label->setScale(75.f / m_label->getContentWidth());
    m_label->setAlignment(kCCTextAlignmentCenter);
    m_label->setID("display-label");
    auto lArrow = CCMenuItemExt::createSpriteExtraWithFrameName("navArrowBtn_001.png", .4f, [this](auto) {
        auto it = std::ranges::find(g_allStyles, m_style);
        if (it == g_allStyles.begin() || it == g_allStyles.end()) it = g_allStyles.end() - 1;
        else --it;
        updateState(*it);
        m_menu->updateState();
    });
    static_cast<CCSprite*>(lArrow->getNormalImage())->setFlipX(true);
    lArrow->setID("left-arrow-button");
    auto rArrow = CCMenuItemExt::createSpriteExtraWithFrameName("navArrowBtn_001.png", .4f, [this](auto) {
        auto it = std::ranges::find(g_allStyles, m_style);
        if (it == g_allStyles.end() - 1 || it == g_allStyles.end()) it = g_allStyles.begin();
        else ++it;
        updateState(*it);
        m_menu->updateState();
    });
    rArrow->setID("right-arrow-button");

    auto del = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_deleteIcon_001.png", .7f, [this](auto) {
        removeFromParent();
        std::erase(m_menu->m_nodes, this);
        m_menu->updateState();
    });
    del->setID("delete-button");

    m_preview = SimplePlayer::create(1);
    m_preview->setID("preview");
    m_preview->setScale(5.f / 6);
    addChildAtPosition(m_preview, Anchor::TopLeft, {15, -15});

    addChildAtPosition(m_label, Anchor::Top, {5.f, -15.f});
    addChildAtPosition(lArrow, Anchor::TopLeft, {35.f, -15.f});
    addChildAtPosition(rArrow, Anchor::TopRight, {-25.f, -15.f});
    addChildAtPosition(del, Anchor::TopRight, {-10.f, -15.f});

    m_customMenu = CCMenu::create();
    m_customMenu->setID("custom-edit-menu");
    m_customMenu->setContentSize({155, 90});
    m_customMenu->setAnchorPoint({.5f, .5f});
    m_customMenu->ignoreAnchorPointForPosition(false);

    auto col1Lab = CCLabelBMFont::create("Color 1", "bigFont.fnt");
    col1Lab->setAnchorPoint({0.f, .5f});
    col1Lab->setID("col1-label");
    col1Lab->setContentSize({120, 90.f});
    col1Lab->setScale(.4f);
    m_customMenu->addChildAtPosition(col1Lab, Anchor::Center, {-45, 30});

    auto col1Spr = ColorChannelSprite::create();
    col1Spr->setColor(m_style.col1);
    col1Spr->setScale(.7f);
    auto col1Btn = CCMenuItemExt::createSpriteExtra(col1Spr, [this](auto) {
        auto popup = ColorPickPopup::create(m_style.col1);
        popup->setCallback([this](ccColor4B const& color) {
            m_style.col1 = ccColor3B(color.r, color.g, color.b);
            updateState(m_style);
            m_menu->updateState();
        });
        popup->show();
    });
    col1Btn->setID("col1");
    m_customMenu->addChildAtPosition(col1Btn, Anchor::Center, {62.5, 30});

    auto col2Lab = CCLabelBMFont::create("Color 2", "bigFont.fnt");
    col2Lab->setAnchorPoint({0.f, .5f});
    col2Lab->setID("col2-label");
    col2Lab->setContentSize({120, 90.f});
    col2Lab->setScale(.4f);
    m_customMenu->addChildAtPosition(col2Lab, Anchor::Center, {-45, 0});

    auto col2Spr = ColorChannelSprite::create();
    col2Spr->setColor(m_style.col2);
    col2Spr->setScale(.7f);
    auto col2Btn = CCMenuItemExt::createSpriteExtra(col2Spr, [this](auto) {
        auto popup = ColorPickPopup::create(m_style.col2);
        popup->setCallback([this](ccColor4B const& color) {
            m_style.col2 = ccColor3B(color.r, color.g, color.b);
            updateState(m_style);
            m_menu->updateState();
        });
        popup->show();
    });
    col2Btn->setID("col2");
    m_customMenu->addChildAtPosition(col2Btn, Anchor::Center, {62.5, 0});

    auto glowLab = CCLabelBMFont::create("Glow", "bigFont.fnt");
    glowLab->setAnchorPoint({0.f, .5f});
    glowLab->setID("glow-label");
    glowLab->setContentSize({120, 90.f});
    glowLab->setScale(.4f);
    m_customMenu->addChildAtPosition(glowLab, Anchor::Center, {-45, -30});

    auto glowSpr = ColorChannelSprite::create();
    glowSpr->setColor(m_style.glow);
    glowSpr->setScale(.7f);
    auto glowBtn = CCMenuItemExt::createSpriteExtra(glowSpr, [this](auto) {
        auto popup = ColorPickPopup::create(m_style.glow);
        popup->setCallback([this](ccColor4B const& color) {
            m_style.glow = ccColor3B(color.r, color.g, color.b);
            updateState(m_style);
            m_menu->updateState();
        });
        popup->show();
    });
    glowBtn->setID("glow");
    m_customMenu->addChildAtPosition(glowBtn, Anchor::Center, {62.5, -30});

    auto toggle = CCMenuItemExt::createTogglerWithStandardSprites(.7f, [this](auto) {
        queueInMainThread([this] {
            m_style.useGlow = !m_style.useGlow;
            updateState(m_style);
            m_menu->updateState();
        });
    });
    toggle->setCascadeOpacityEnabled(true);
    glowBtn->setEnabled(m_style.useGlow);
    toggle->setID("glow-toggler");
    m_customMenu->addChildAtPosition(toggle, Anchor::Center, {32.5, -30});

    addChildAtPosition(m_customMenu, Anchor::Center, {0, -15.f});
    return true;
}

void StyleNode::updateState(const Style& style) {
    m_style = std::move(style);
    m_label->setString(m_style.type.c_str());
    if (auto width = m_label->getContentWidth(); width > 0.001f) m_label->setScale(std::min(80.f / width, .45f));
    else m_label->setScale(.001f);
    setContentHeight(m_style.type == "Custom" ? 120.f : 30.f);
    m_customMenu->setVisible(m_style.type == "Custom");

    if (auto spr = static_cast<ColorChannelSprite*>(m_customMenu->getChildByID("col1")->getChildByIndex(0))) spr->setColor(m_style.col1);
    if (auto spr = static_cast<ColorChannelSprite*>(m_customMenu->getChildByID("col2")->getChildByIndex(0))) spr->setColor(m_style.col2);
    auto glowBtn = m_customMenu->getChildByID("glow");
    if (auto spr = static_cast<ColorChannelSprite*>(glowBtn->getChildByIndex(0))) spr->setColor(m_style.glow);
    typeinfo_cast<CCMenuItemSpriteExtra*>(glowBtn)->setEnabled(m_style.useGlow);
    typeinfo_cast<CCMenuItemToggler*>(m_customMenu->getChildByID("glow-toggler"))->toggle(m_style.useGlow);
    updateLayout();

    m_preview->setColors(style.getColor1(), style.getColor2());
    if (style.isGlowEnabled()) m_preview->setGlowOutline(style.getGlowColor());
    else m_preview->disableGlowOutline();
    if (m_style.type == "Cosmic Mario\n(SMG 1)") {
        CosmicClone::updateShaderForPlayer(m_preview, ShaderManager::get().getCosmicShader());
        CosmicClone::updateSpriteForPlayer(m_preview, nullptr);
    } else if (m_style.type == "Cosmic Clone\n(SMG 2)") {
        CosmicClone::updateShaderForPlayer(m_preview, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
        CosmicClone::updateSpriteForPlayer(m_preview, nullptr);
    } else if (m_style.type == "Badeline Chaser\n(Celeste)") {
        CosmicClone::updateShaderForPlayer(m_preview, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
        CosmicClone::updateSpriteForPlayer(m_preview, nullptr);
    } else if (m_style.type == "Hungry Luma") {
        CosmicClone::updateShaderForPlayer(m_preview, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
        CosmicClone::updateSpriteForPlayer(m_preview, CCSprite::create("HungryLuma.png"_spr));
    } else if (m_style.type == "The Yellow One") {
        CosmicClone::updateShaderForPlayer(m_preview, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
        CosmicClone::updateSpriteForPlayer(m_preview, CCSprite::create("MD_DifficultyYOSmall.png"_spr));
    } else if (m_style.type == "eri") {
        CosmicClone::updateShaderForPlayer(m_preview, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
        CosmicClone::updateSpriteForPlayer(m_preview, CCSprite::create("eri.png"_spr), {1.27f, 1.f});
    } else if (m_style.type == "Markiplier") {
        CosmicClone::updateShaderForPlayer(m_preview, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
        CosmicClone::updateSpriteForPlayer(m_preview, CCSprite::create("markiplier.png"_spr));
    } else {
        CosmicClone::updateShaderForPlayer(m_preview, CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
        CosmicClone::updateSpriteForPlayer(m_preview, nullptr);
    }
}

StyleNode* StyleNode::create(StylePickMenu* menu, const Style& value) {
    auto ret = new StyleNode();
    if (ret->init(menu, value)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

$on_mod(Loaded) {
    ObjectAPI::registerObject(ObjectInfo::builder()
        .id("cosmic-clones-trigger"_spr)
        .sprite("triggerMain.png"_spr)
        .construction(ComplexObject::builder()
            .factory(CosmicClonesTrigger::create)
            .customProperties({
                PropertyInterface::from(109, &CosmicClonesTrigger::m_count, 3),
                PropertyInterface::from(119, &CosmicClonesTrigger::m_initialDelay, 1.75),
                PropertyInterface::from(124, &CosmicClonesTrigger::m_delay, 1),
                PropertyInterface::from(208, &CosmicClonesTrigger::m_disabled, false),
                PropertyInterface::from(51,  &CosmicClonesTrigger::m_controllerId, 1),
                PropertyInterface::from(130, &CosmicClonesTrigger::m_damage, true),
                PropertyInterface::from(140, &CosmicClonesTrigger::m_stopper, false),
                PropertyInterface::from(158, &CosmicClonesTrigger::m_sfx, true),
                PropertyInterface::from(168, &CosmicClonesTrigger::m_styles, std::vector<Style>{{"Cosmic Mario\n(SMG 1)"}}),
            })
            .build())
        .editObject(CosmicClonesTrigger::getEditObjectConfig)
        .editorTab(EditorTab::Triggers)
        .build());
}