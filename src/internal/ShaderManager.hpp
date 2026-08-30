#pragma once

// inheriting ccobject for scheduler update rip
class ShaderManager : public cocos2d::CCObject {
protected:
    cocos2d::CCTexture2D* m_cosmicTex = 0;
    cocos2d::CCTexture2D* m_normalTex = 0;
    cocos2d::CCTexture2D* m_overlayTex = 0;

    float m_time = 0;
    bool m_hasSetup = false;

    cocos2d::CCGLProgram* m_shader = nullptr;
public:
    static ShaderManager& get();
    cocos2d::CCGLProgram* getCosmicShader() const;
    void setup();
    void update(float dt);
};
