#pragma once

#include <Geode/modify/PlayerObject.hpp>

#include "../ui/CloneStyleSetting.hpp"

enum class Animation {
    Jump,
    Fall,
    Idle,
    Run,
    None
};

struct Snapshot {
    cocos2d::CCPoint pos;
    float rotation;
    float scale;
    bool left = false;
    bool side = false;
    bool visible;
    IconType type;
    bool flip;
    Animation anim = Animation::None;

    // p2
    bool dualEnabled = false;
    cocos2d::CCPoint pos2;
    float rotation2;
    float scale2;
    bool left2 = false;
    bool side2 = false;
    bool visible2;
    IconType type2;
    bool flip2;
    Animation anim2 = Animation::None;
};

class CosmicClone;

class $modify(CosmicPlayerObject, PlayerObject) {
    struct Fields {
        bool m_isCosmic = false;
        CosmicClone* m_clone = nullptr;
    };

    static CosmicPlayerObject* createCosmic(int player, int ship, GJBaseGameLayer* gameLayer, cocos2d::CCLayer* layer, bool playLayer);
};

enum class CosmicCloneSFXType {
    FirstSpawn,
    Spawn,
    KillPlayer,
    Die,
};

class CosmicClone {
protected:
    int m_delay = 0;
    CosmicPlayerObject* m_p1 = nullptr;
    CosmicPlayerObject* m_p2 = nullptr;
    Style m_style;

    bool m_dual = false;
    IconType m_p1type = IconType::Cube;
    IconType m_p2type = IconType::Cube;
    Animation m_p1anim = Animation::None;
    Animation m_p2anim = Animation::None;
    float m_sillyScale = 1.f;

    void init(int delay, bool plat, GJBaseGameLayer* gjbgl);
public:
    static std::shared_ptr<CosmicClone> create(int delay, bool plat, GJBaseGameLayer* gjbgl);
    CosmicPlayerObject* getP1() const;
    CosmicPlayerObject* getP2() const;
    int getDelay() const;
    void checkCollision(PlayerObject* player) const;
    void setDual(bool dual);
    IconType getType(int player) const;
    void setType(IconType type, int player);
    Animation getAnimation(int player) const;
    void animate(Animation animation, int player);
    void updateAnimation(int player) const;

    void setScale1(float scale);
    void setScale2(float scale);
    void setPos1(cocos2d::CCPoint pos);
    void setPos2(cocos2d::CCPoint pos);

    int playSFX(CosmicCloneSFXType type) const;

    void updateStyle(Style style);
    void remove();
    static void removePlayer(PlayerObject* player);
    static void updateShaderForPlayer(PlayerObject* player, cocos2d::CCGLProgram* shader, bool applyToRobotSprites = false);
    static void updateSpriteForPlayer(PlayerObject* player, cocos2d::CCSprite* sprite, const cocos2d::CCPoint& scaleMult = {1.f, 1.f});
    static void updateShaderForPlayer(SimplePlayer* player, cocos2d::CCGLProgram* shader, bool applyToRobotSprites = false);
    static void updateSpriteForPlayer(SimplePlayer* player, cocos2d::CCSprite* sprite, const cocos2d::CCPoint& scaleMult = {1.f, 1.f});
};
