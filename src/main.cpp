#include <Geode/Geode.hpp>
#include <smjs.object-collab/include/object_collab.hpp>

#include "BigPortal/BigPortal.hpp"

using namespace geode::prelude;
using namespace object_collab::prelude;

$on_mod(Loaded) {
    ObjectAPI::registerObject(
        ObjectInfo::builder()
            .id(BigPortal::STRING_ID)
            .sprite(BigPortal::FRAME_FRONT)
            .editorTab(EditorTab::PlayerModifiers)
            .editorButtonColor(EditorButtonColor::DarkGray)
            .construction(ComplexObject::builder().factory(BigPortal::create).build())
            .editSpecial(BigPortal::getEditSpecialConfig)
            .build());
}

// $on_game(Loaded) {
//     auto levelManager = LocalLevelManager::get();
//     if (!levelManager) {
//         log::error("Failed to get LocalLevelManager instance.");
//         return;
//     }

//     if (levelManager->m_localLevels) {
//         auto level = static_cast<GJGameLevel*>(levelManager->m_localLevels->objectAtIndex(0));
//         auto scene = LevelEditorLayer::scene(level, false);
//         CCDirector::sharedDirector()->pushScene(scene);
//     }
// };
