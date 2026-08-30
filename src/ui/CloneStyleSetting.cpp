#include "CloneStyleSetting.hpp"

using namespace geode::prelude;

ccColor3B Style::getColor1() const {
    if (type == "Cosmic Clone\n(SMG 2)") return {60, 20, 21};
    if (type == "Badeline Chaser\n(Celeste)") return {155, 63, 181};
    if (type == "Custom") return col1;
    if (type == "Hungry Luma" || type == "The Yellow One" || type == "eri" || type == "Markiplier") return {255, 255, 255};
    return {12, 11, 56};
}

ccColor3B Style::getColor2() const {
    if (type == "Cosmic Clone\n(SMG 2)") return {243, 235, 87};
    if (type == "Badeline Chaser\n(Celeste)") return {191, 29, 51};
    if (type == "Custom") return col2;
    if (type == "Hungry Luma" || type == "The Yellow One" || type == "eri" || type == "Markiplier") return {255, 255, 255};
    return {11, 27, 56};
}

bool Style::isGlowEnabled() const {
    if (type == "Badeline Chaser\n(Celeste)") return false;
    if (type == "Custom") return useGlow;
    if (type == "Hungry Luma" || type == "The Yellow One" || type == "eri" || type == "Markiplier") return false;
    return true;
}

ccColor3B Style::getGlowColor() const {
    if (type == "Cosmic Clone\n(SMG 2)") return {193, 50, 54};
    if (type == "Badeline Chaser\n(Celeste)") return ccWHITE;
    if (type == "Custom") return glow;
    return {13, 23, 64};
}

bool Style::operator==(const Style& other) const {
    if (type != other.type) return false;
    if (type != "Custom") return true;
    return col1 == other.col1 && col2 == other.col2 && useGlow == other.useGlow && (!useGlow || glow == other.glow);
}

bool Style::operator!=(const Style& other) const {
    return !(*this == other);
}


Result<std::shared_ptr<SettingV3>> CloneStyleSetting::parse(std::string const &key, std::string const &modID, matjson::Value const &json) {
    auto res = std::make_shared<CloneStyleSetting>();
    auto root = checkJson(json, "CloneStyleSetting");
    res->parseBaseProperties(key, modID, root);
    root.has("styles").into(res->m_styles);
    root.checkUnknownKeys();
    return root.ok(std::static_pointer_cast<SettingV3>(res));
}

SettingNodeV3* CloneStyleSetting::createNode(float width) {
    return CloneStyleSettingNode::create(std::static_pointer_cast<CloneStyleSetting>(shared_from_this()), width);
}

void CloneStyleSettingNode::updateState(CCNode *invoker) {
    SettingValueNodeV3::updateState(invoker);
    auto val = getValue();
    auto bm = getButtonMenu();
    typeinfo_cast<AnchorLayoutOptions*>(getNameMenu()->getLayoutOptions())->setOffset({10.f, getContentHeight() / 2 - 15.f});
    if (m_nodes.size() < val.size()) {
        for (size_t i = m_nodes.size(); i < val.size(); ++i) {
            auto node = StyleSettingNode::create(this, val[i]);
            node->setTag(static_cast<int>(i));
            node->getChildByID("delete-button")->setVisible(val.size() > 1);
            m_nodes.push_back(node);
            bm->addChild(node);
        }
    }
    for (size_t i = 0; i < m_nodes.size();) {
        auto node = m_nodes[i];
        if (i >= val.size()) {
            node->removeFromParent();
            m_nodes.erase(m_nodes.begin() + i);
        } else {
            node->setTag(static_cast<int>(i));
            node->getChildByID("delete-button")->setVisible(val.size() > 1);
            node->updateState(val[i]);
            ++i;
        }
    }

    m_addBtn->setVisible(val.size() < 20);

    auto h = 30.f;
    for (auto node : m_nodes) {
        h += node->getContentHeight();
    }
    bm->setContentHeight(h);
    setContentHeight(h);
    bm->updateLayout();

    auto enable = getSetting()->shouldEnable();
    getButtonMenu()->setTouchEnabled(enable);
    getButtonMenu()->setOpacity(enable ? 255 : 127);
}

bool CloneStyleSettingNode::init(std::shared_ptr<CloneStyleSetting> setting, float width)  {
    if (!SettingValueNodeV3::init(setting, width))
        return false;

    auto bm = getButtonMenu();
    m_addBtn = CCMenuItemExt::createSpriteExtraWithFrameName("edit_addCBtn_001.png", .5f, [this](auto btn) {
        auto val = getValue();
        val.push_back(Style{"Cosmic Mario\n(SMG 1)"});
        setValue(val, btn);
    });
    m_addBtn->setContentSize({30.f, 30.f});
    auto img = m_addBtn->getNormalImage();
    img->removeFromParent();
    m_addBtn->addChildAtPosition(img, Anchor::Center);
    bm->addChild(m_addBtn);
    int i = 0;
    auto val = getValue();
    for (auto value : val) {
        auto node = StyleSettingNode::create(this, value);
        node->setTag(i);
        node->getChildByID("delete-button")->setVisible(val.size() > 1);
        m_nodes.push_back(node);
        bm->addChild(node);
        i++;
    }

    m_addBtn->setVisible(val.size() < 20);

    bm->setContentWidth(100);
    bm->setLayout(ColumnLayout::create()->setAxisReverse(true)->setGap(0.f));

    this->updateState(nullptr);

    return true;
}

void CloneStyleSettingNode::setNodeValue(StyleSettingNode* node, Style value) {
    auto val = getValue();
    val[node->getTag()] = value;
    setValue(val, node);
}

void CloneStyleSettingNode::remove(int id) {
  auto val = getValue();
  if (id < 0 || id >= static_cast<int>(val.size()))
    return;
  val.erase(val.begin() + id);
  setValue(val, this);
}


CloneStyleSettingNode* CloneStyleSettingNode::create(std::shared_ptr<CloneStyleSetting> setting, float width) {
    auto ret = new CloneStyleSettingNode();
    if (ret->init(setting, width)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool StyleSettingNode::init(CloneStyleSettingNode* setting, Style value) {
    if (!CCMenu::init()) return false;
    setID("clone-style-node");
    m_setting = setting;
    m_style = std::move(value);
    ignoreAnchorPointForPosition(false);
    setAnchorPoint({.5f, .5f});
    setContentSize({120, m_style.type == "Custom" ? 120.f : 30.f});
    m_label = CCLabelBMFont::create(value.type.c_str(), "bigFont.fnt");
    m_label->setScale(75.f / m_label->getContentWidth());
    m_label->setAlignment(kCCTextAlignmentCenter);
    m_label->setID("display-label");
    auto lArrow = CCMenuItemExt::createSpriteExtraWithFrameName("navArrowBtn_001.png", .4f, [this](auto) {
        auto it = std::find(g_allStyles.begin(), g_allStyles.end(), m_style);
        if (it == g_allStyles.begin() || it == g_allStyles.end())
            it = g_allStyles.end() - 1;
        else
            --it;
        auto val = *it;
        m_setting->setNodeValue(this, val);
    });
    static_cast<CCSprite*>(lArrow->getNormalImage())->setFlipX(true);
    lArrow->setID("left-arrow-button");
    auto rArrow = CCMenuItemExt::createSpriteExtraWithFrameName("navArrowBtn_001.png", .4f, [this, value](auto) {
        auto it = std::find(g_allStyles.begin(), g_allStyles.end(), m_style);
        if (it == g_allStyles.end() - 1 || it == g_allStyles.end())
            it = g_allStyles.begin();
        else
            ++it;
        auto val = *it;
        m_setting->setNodeValue(this, val);
    });
    rArrow->setID("right-arrow-button");

    auto del = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_deleteIcon_001.png", .7f, [this, value](auto) {
        m_setting->remove(getTag());
    });
    del->setID("delete-button");

    addChildAtPosition(m_label, Anchor::Top, {-7.5f, -15.f});
    addChildAtPosition(lArrow, Anchor::TopLeft, {5.f, -15.f});
    addChildAtPosition(rArrow, Anchor::TopRight, {-20.f, -15.f});
    addChildAtPosition(del, Anchor::TopRight, {-5.f, -15.f});

    m_customMenu = CCMenu::create();
    m_customMenu->setAnchorPoint({.5f, .5f});
    m_customMenu->ignoreAnchorPointForPosition(false);

    auto col1Lab = CCLabelBMFont::create("Color 1", "bigFont.fnt");
    col1Lab->setAnchorPoint({0.f, .5f});
    col1Lab->setContentSize({120, 90.f});
    col1Lab->setScale(.4f);
    m_customMenu->addChildAtPosition(col1Lab, Anchor::Left, {19, 30});

    auto col1Spr = ColorChannelSprite::create();
    col1Spr->setColor(m_style.col1);
    col1Spr->setScale(.7f);
    auto col1Btn = CCMenuItemExt::createSpriteExtra(col1Spr, [this](auto) {
        auto popup = ColorPickPopup::create(m_style.col1);
        popup->setCallback([this](ccColor4B const& color) {
            m_style.col1 = ccColor3B(color.r, color.g, color.b);
            m_setting->setNodeValue(this, m_style);
        });
        popup->show();
    });
    col1Btn->setID("col1");
    m_customMenu->addChildAtPosition(col1Btn, Anchor::Center, {50, 30});

    auto col2Lab = CCLabelBMFont::create("Color 2", "bigFont.fnt");
    col2Lab->setAnchorPoint({0.f, .5f});
    col2Lab->setContentSize({120, 90.f});
    col2Lab->setScale(.4f);
    m_customMenu->addChildAtPosition(col2Lab, Anchor::Left, {19, 0});

    auto col2Spr = ColorChannelSprite::create();
    col2Spr->setColor(m_style.col2);
    col2Spr->setScale(.7f);
    auto col2Btn = CCMenuItemExt::createSpriteExtra(col2Spr, [this](auto) {
        auto popup = ColorPickPopup::create(m_style.col2);
        popup->setCallback([this](ccColor4B const& color) {
            m_style.col2 = ccColor3B(color.r, color.g, color.b);
            m_setting->setNodeValue(this, m_style);
        });
        popup->show();
    });
    col2Btn->setID("col2");
    m_customMenu->addChildAtPosition(col2Btn, Anchor::Center, {50, 0});

    auto glowLab = CCLabelBMFont::create("Glow", "bigFont.fnt");
    glowLab->setAnchorPoint({0.f, .5f});
    glowLab->setContentSize({120, 90.f});
    glowLab->setScale(.4f);
    m_customMenu->addChildAtPosition(glowLab, Anchor::Left, {19, -30});

    auto glowSpr = ColorChannelSprite::create();
    glowSpr->setColor(m_style.glow);
    glowSpr->setScale(.7f);
    auto glowBtn = CCMenuItemExt::createSpriteExtra(glowSpr, [this](auto) {
        auto popup = ColorPickPopup::create(m_style.glow);
        popup->setCallback([this](ccColor4B const& color) {
            m_style.glow = ccColor3B(color.r, color.g, color.b);
            m_setting->setNodeValue(this, m_style);
        });
        popup->show();
    });
    glowBtn->setID("glow");
    m_customMenu->addChildAtPosition(glowBtn, Anchor::Center, {50, -30});

    auto toggle = CCMenuItemExt::createTogglerWithStandardSprites(.7f, [this, glowBtn](auto) {
        queueInMainThread([this] {
            m_style.useGlow = !m_style.useGlow;
            m_setting->setNodeValue(this, m_style);
        });
    });
    toggle->setCascadeOpacityEnabled(true);
    glowBtn->setEnabled(m_style.useGlow);
    toggle->setID("glow-toggler");
    m_customMenu->addChildAtPosition(toggle, Anchor::Center, {20, -30});

    addChildAtPosition(m_customMenu, Anchor::Center, {0.f, -15.f});
    return true;
}

void StyleSettingNode::updateState(const Style& style) {
    m_style = std::move(style);
    m_label->setString(m_style.type.c_str());
    if (auto width = m_label->getContentWidth(); width > 0.001f) m_label->setScale(std::min(80.f / width, .45f));
    else m_label->setScale(.001f);
    setContentHeight(m_style.type == "Custom" ? 120.f : 30.f);
    m_customMenu->setVisible(m_style.type == "Custom");

    m_customMenu->getChildByID("col1")->getChildByType<ColorChannelSprite*>(0)->setColor(m_style.col1);
    m_customMenu->getChildByID("col2")->getChildByType<ColorChannelSprite*>(0)->setColor(m_style.col2);
    auto glowBtn = m_customMenu->getChildByID("glow");
    glowBtn->getChildByType<ColorChannelSprite*>(0)->setColor(m_style.glow);
    typeinfo_cast<CCMenuItemSpriteExtra*>(glowBtn)->setEnabled(m_style.useGlow);
    typeinfo_cast<CCMenuItemToggler*>(m_customMenu->getChildByID("glow-toggler"))->toggle(m_style.useGlow);
    bool enable = m_setting->getSetting()->shouldEnable();
    setTouchEnabled(enable);
    m_customMenu->setTouchEnabled(enable);
    updateLayout();
}

StyleSettingNode* StyleSettingNode::create(CloneStyleSettingNode* setting, const Style& value) {
    auto ret = new StyleSettingNode();
    if (ret->init(setting, value)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

$execute {
    (void)Mod::get()->registerCustomSettingType("clone-style", &CloneStyleSetting::parse);
}