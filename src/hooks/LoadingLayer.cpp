#include <Geode/modify/LoadingLayer.hpp>

using namespace geode::prelude;

class $modify(LoadingLayer) {
    bool init(bool a1) {
        std::filesystem::path zipFilePath = Mod::get()->getResourcesDir() / "RandomPortals.zip";
        std::filesystem::path unzipDir = Mod::get()->getResourcesDir() / "RandomPortals";

        geode::utils::file::Unzip::intoDir(zipFilePath, unzipDir);

        auto unzipDirStr = geode::utils::string::pathToString(unzipDir);

        CCFileUtils::get()->addTexturePack(CCTexturePack{
            .m_id = this->getID(),
            .m_paths = { unzipDirStr }
            });

        if (!LoadingLayer::init(a1)) return false;

        auto GM = GameManager::sharedState();

        if (!GM->getUGV("30")) {
            GM->setUGV("30", true);
        }

        return true;
    }
};

