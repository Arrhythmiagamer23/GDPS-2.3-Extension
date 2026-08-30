// see PlayerObject::propellPlayer

// #include <Geode/modify/GJBaseGameLayer.hpp>
// class $modify(MyGJBaseGameLayer, GJBaseGameLayer) {

// void bumpPlayer(PlayerObject *player, EffectGameObject *object)
// {
//     auto myPlayer = static_cast<MyPlayerObject *>(player);
//     if (!myPlayer || !myPlayer->isBig())
//     {
//         GJBaseGameLayer::bumpPlayer(player, object);
//         return;
//     }

//     if (canBeActivatedByPlayer(player, object))
//         return;

//     // why???
//     player->m_lastPortalPos = object->getPosition() + CCPoint{0.f, -10.f};

//     float bumpMod = 1.f;

//     if (object->getType() == GameObjectType::PinkJumpPad)
//     {
//         if (player->m_isShip)
//             bumpMod = .35f;
//         else if (player->m_isBird)
//             bumpMod = .4f;
//         else if (player->m_isBall)
//             bumpMod = .7f;
//         else if (player->m_isSpider)
//             bumpMod = .7f;
//         else
//             bumpMod = .65f;
//     }
//     else if (object->getType() == GameObjectType::RedJumpPad)
//     {
//         if (player->m_isShip)
//             bumpMod = player->m_vehicleSize >= 1.0 ? .63f : .95f;
//         else if (player->m_isBird)
//             bumpMod = player->m_vehicleSize >= 1.0 ? .6f : .98f;
//         else
//             bumpMod = 1.25f;
//     }

//     player->m_lastActivatedPortal = object;

//     if (object->m_isReverse)
//         player->reversePlayer(object);

//     player->bumpPlayer(bumpMod, (int)object->getType(), object->m_hasNoEffects, object);

//     gameEventTriggered(objectTypeToGameEvent((int)object->getType()), 0, 0);
//     gameEventTriggered(GJGameEvent::PadActivated, 0, 0);
// }
// };
