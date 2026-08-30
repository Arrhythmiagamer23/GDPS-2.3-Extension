#include "ShaderManager.hpp"

#include <Geode/loader/SettingV3.hpp>

#include "../hooks/Gameplay.hpp"
#include "Shaders.hpp"

using namespace geode::prelude;

ShaderManager& ShaderManager::get() {
    static ShaderManager instance;
    return instance;
}

CCGLProgram* ShaderManager::getCosmicShader() const {
    return m_shader;
}

void ShaderManager::setup() {
    m_hasSetup = true;
    m_shader = new CCGLProgram;
    if (!m_shader->initWithVertexShaderByteArray(g_cosmicVsh, g_cosmicFsh)) {
        log::error("Cosmic shader failed to load!\n{}\n{}", m_shader->fragmentShaderLog(), m_shader->vertexShaderLog()); // probably going to crash anyway
        delete m_shader;
        return;
    }

    m_shader->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
    m_shader->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
    m_shader->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);

    m_shader->link();
    m_shader->updateUniforms();

    m_shader->setUniformLocationWith1i(m_shader->getUniformLocationForName("CC_Texture1"), 1);
    m_shader->setUniformLocationWith1i(m_shader->getUniformLocationForName("CC_Texture2"), 2);
    m_shader->setUniformLocationWith1i(m_shader->getUniformLocationForName("CC_Texture3"), 3);

    CCShaderCache::sharedShaderCache()->addProgram(m_shader, "cosmic"_spr);

    m_cosmicTex = CCTextureCache::get()->addImage("cosmic.png"_spr, false);
    m_normalTex = CCTextureCache::get()->addImage("normal.png"_spr, false);
    m_overlayTex = CCTextureCache::get()->addImage("star.png"_spr, false);
}

void ShaderManager::update(const float dt) {
    if (!m_hasSetup) setup();
    if (!m_shader) return;
    m_shader->use();
    m_shader->setUniformsForBuiltins();

    ccGLBindTexture2DN(1, m_cosmicTex->getName());
    ccGLBindTexture2DN(2, m_normalTex->getName());
    ccGLBindTexture2DN(3, m_overlayTex->getName());

    m_time += dt;
    auto uTime = m_shader->getUniformLocationForName("u_time");
    m_shader->setUniformLocationWith1f(uTime, m_time);

    auto size = CCDirector::sharedDirector()->getVisibleSize();
    auto uScreenSize = m_shader->getUniformLocationForName("u_screenSize");
    m_shader->setUniformLocationWith2f(uScreenSize, size.width, size.height);
}

$on_mod(Loaded) {
    CCScheduler::get()->scheduleUpdateForTarget(&ShaderManager::get(), 0, false);
}
