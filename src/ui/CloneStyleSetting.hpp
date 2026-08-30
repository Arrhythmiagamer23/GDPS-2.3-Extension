#pragma once

#include <Geode/loader/SettingV3.hpp>

struct Style {
    Style() {};
    Style(const std::string &type) : type(type) {}
    Style(const std::string &type, const cocos2d::ccColor3B col1, const cocos2d::ccColor3B col2, const bool useGlow, const cocos2d::ccColor3B glow) : type(type), col1(col1), col2(col2), useGlow(useGlow), glow(glow) {}

    std::string type = "Cosmic Mario\n(SMG 1)";
    cocos2d::ccColor3B col1 = cocos2d::ccWHITE;
    cocos2d::ccColor3B col2 = cocos2d::ccWHITE;
    bool useGlow = false;
    cocos2d::ccColor3B glow = cocos2d::ccWHITE;

    cocos2d::ccColor3B getColor1() const;
    cocos2d::ccColor3B getColor2() const;
    cocos2d::ccColor3B getGlowColor() const;
    bool isGlowEnabled() const;

    bool operator==(const Style &other) const;

    bool operator!=(const Style &other) const;
};

template<>
struct matjson::Serialize<Style> {
    static geode::Result<Style> fromJson(const Value &value) {
        GEODE_UNWRAP_INTO(std::string type, value["type"].asString());
        if (type == "Custom") {
            GEODE_UNWRAP_INTO(cocos2d::ccColor3B col1, value["col1"].as<cocos2d::ccColor3B>());
            GEODE_UNWRAP_INTO(cocos2d::ccColor3B col2, value["col2"].as<cocos2d::ccColor3B>());
            GEODE_UNWRAP_INTO(bool useGlow, value["use-glow"].as<bool>());
            GEODE_UNWRAP_INTO(cocos2d::ccColor3B glow, value["glow"].as<cocos2d::ccColor3B>());
            return geode::Ok(Style{type, col1, col2, useGlow, glow});
        }
        return geode::Ok(Style{type});
    }

    static Value toJson(const Style &style) {
        if (style.type == "Custom")
            return makeObject({
                {"type", style.type},
                {"col1", style.col1},
                {"col2", style.col2},
                {"use-glow", style.useGlow},
                {"glow", style.glow},
            });
        return makeObject({{"type", style.type}});
    }
};

static std::vector<Style> g_allStyles {
    {"Cosmic Mario\n(SMG 1)"},
    {"Cosmic Clone\n(SMG 2)"},
    {"Badeline Chaser\n(Celeste)"},
    {"Hungry Luma"},
    {"The Yellow One"},
    {"eri"},
    {"Markiplier"},
    {"Custom"},
};

class CloneStyleSetting : public geode::SettingBaseValueV3<std::vector<Style> > {
protected:
    std::vector<Style> m_styles = {};

public:
    static geode::Result<std::shared_ptr<SettingV3> > parse(std::string const &key, std::string const &modID,
                                                            matjson::Value const &json);

    geode::SettingNodeV3 *createNode(float width) override;
};

template<>
struct geode::SettingTypeForValueType<std::vector<Style> > {
    using SettingType = CloneStyleSetting;
};

class StyleSettingNode;

class CloneStyleSettingNode : public geode::SettingValueNodeV3<CloneStyleSetting> {
protected:
    bool init(std::shared_ptr<CloneStyleSetting> setting, float width);

    void updateState(CCNode *invoker) override;

    std::vector<StyleSettingNode *> m_nodes;
    CCMenuItemSpriteExtra *m_addBtn = nullptr;

public:
    void setNodeValue(StyleSettingNode *node, Style value);

    void remove(int id);

    static CloneStyleSettingNode *create(std::shared_ptr<CloneStyleSetting> setting, float width);
};


class StyleSettingNode : public cocos2d::CCMenu {
protected:
    bool init(CloneStyleSettingNode *setting, Style value);

    Style m_style;
    cocos2d::CCLabelBMFont *m_label = nullptr;
    CloneStyleSettingNode *m_setting = nullptr;
    CCMenu *m_customMenu = nullptr;

public:
    void updateState(const Style &style);

    const Style &getStyle() { return m_style; }

    static StyleSettingNode *create(CloneStyleSettingNode *setting, const Style &value);
};
