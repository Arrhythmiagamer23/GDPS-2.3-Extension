#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>

using namespace geode::prelude;

class $modify(OdysseyGameManager, GameManager)
{
    struct Fields {
        int m_loadedBGIdx = 0;
        int m_loadedGIdx = 0;
    };
    
    bool init() {
        if (!GameManager::init()) {
            return false;
        }
        
        return true;
    }
    
    void GameManager::loadBackground(int backID)
    {
	    int bgID = backID;
	    if (190 < backID) {
            bgID = 191;
        }
	    if (backID < 1) {
        bgID = 1;
        }
        if (bgID != m_loadedBGIdx) {
		    CCString* bgStr;
		    CCTextureCache* pTextureCache = CCTextureCache::sharedTextureCache();
		    if (this->m_loadedBGIdx != 0)
		    {
			    bgStr = CCString::createWithFormat("game_bg_%02d_001.png", bgID);
			    // pTextureCache->removeTextureForKey(bgStr->getCString);
		    }
		    bgStr = CCString::createWithFormat("game_bg_%02d_001.png", bgID);
		    pTextureCache->addImage(bgStr->getCString());
		    this->m_loadedBGIdx = bgID;
        }
    }

    void GameManager::loadGround(int gID)
    {
        int groundID = gID;
        if (40 < gID) {
            groundID = 41;
        }
        if (gID < 1) {
            groundID = 1;
        }
        if (groundID != this->m_loadedGIdx)
        {
            CCString* groundStr;
            CCTextureCache* pTextureCache = CCTextureCache::sharedTextureCache();
            if (this->m_loadedGIdx != 0)
            {
                groundStr = CCString::createWithFormat("groundSquare_%02d_001.png", groundID);
                // MISSING CC FUNCTION: pTextureCache->removeTextureForKey(groundStr->getCString);
            }
            groundStr = CCString::createWithFormat("groundSquare_%02d_001.png", groundID);
            pTextureCache->addImage(groundStr->getCString());
            this->m_loadedGIdx = groundID;
        }
    }

    // m_id in these 2 functions is a temporary variable used as a failsafe for overshooting on ground and background ids
    char const* GameManager::getBGTexture(int bgID)
    {
        int m_id = bgID;
        if (190 < bgID) {
            m_id = 191;
        }
        if (bgID < 1) {
            m_id = 1;
        }
	    this->loadBackground(m_id);
	    return CCString::createWithFormat("game_bg_%02d_001.png", m_id)->getCString();
    }

    char const* GameManager::getGTexture(int gID)
    {
        int m_id = gID;
        if (40 < gID) {
            m_id = 41;
        }
        if (gID < 1) {
            m_id = 1;
        }
        this->loadGround(m_id);
        return CCString::createWithFormat("groundSquare_%02d_001.png", m_id)->getCString();
    }
};
