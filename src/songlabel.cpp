#include <Geode/modify/PlayLayer.hpp>
#include <Geode/binding/LevelTools.hpp>

using namespace geode::prelude;

class $modify(SongLabelPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* songLabel = nullptr;
    };

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        if (!Mod::get()->getSettingValue<bool>("enable-songlabel")) return;

        auto songInfo = MusicDownloadManager::sharedState()->getSongInfoObject(m_level->m_songID);
        std::string songName;
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float margin = 20.f;
        auto fields = m_fields.self();

        if (Mod::get()->getSettingValue<bool>("songlabel-showartist")) {
            if (songInfo) {
                songName = fmt::format("♪~ {} - {}", songInfo->m_artistName, songInfo->m_songName);
            } else {
                songName = fmt::format("♪~ {}", m_level->getSongName());
                log::error("couldn't find song info! can't show the artist name :(");
            }
        } else {
            songName = fmt::format("♪~ {}", m_level->getSongName());
        }

        fields->songLabel = CCLabelBMFont::create(songName.c_str(), "MusicTitleFont.fnt"_spr);
        fields->songLabel->setAnchorPoint({1.f, 1.f});
        fields->songLabel->getTexture()->setAliasTexParameters();
        fields->songLabel->setPosition({winSize.width - margin, winSize.height - margin * 2});
        fields->songLabel->setScale(1.25f);
        fields->songLabel->setOpacity(0);

        if (m_uiLayer) m_uiLayer->addChild(fields->songLabel, 1000);
    }

    void startGame() {
        PlayLayer::startGame();
        if (!Mod::get()->getSettingValue<bool>("enable-songlabel")) return;

        auto fields = m_fields.self();
        auto labelPos = fields->songLabel->getPosition();
        float moveBy = 35.f;

        auto moveIn = CCSpawn::create(
            CCEaseInOut::create(CCMoveTo::create(1.5f, {labelPos.x - moveBy, labelPos.y}), 2.f),
            CCEaseInOut::create(CCFadeIn::create(1.5f), 2.f),
            nullptr
        );
        auto moveAway = CCSpawn::create(
            CCEaseInOut::create(CCMoveTo::create(1.5f, {labelPos.x, labelPos.y}), 2.f),
            CCEaseInOut::create(CCFadeOut::create(1.5f), 2.f),
            nullptr
        );

        // 3.25
        auto animAction = CCSequence::create(
            moveIn,
            CCDelayTime::create(3.f),
            moveAway,
            nullptr
        );
        
        fields->songLabel->runAction(animAction);
    }
};