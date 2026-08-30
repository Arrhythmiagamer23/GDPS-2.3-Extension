#pragma once

#include <Geode/Geode.hpp>
#include <smjs.object-collab/include/object_collab.hpp>

using namespace geode::prelude;
using namespace object_collab::prelude;

class $object(BigPortal, EffectGameObject) {
   public:
    static const ccColor3B EFFECT_COLOR;

    static const std::string FRAME_BACK;

    static const std::string FRAME_FRONT;

    static const float PLAYER_SIZE;

    static const std::string STRING_ID;

   public:
    static BigPortal* create(ObjectInfo* info);

    static PopupOptions getEditSpecialConfig(const Selected& selected);

    BigPortal(ObjectInfo* info)
        : CustomObject(info, ObjectTraits::builder()
                                 .gameObjectType(GameObjectType::MiniSizePortal)
                                 .defaultMainColorID(0)
                                 .defaultZLayer(ZLayer::T1)
                                 .onPlayShineEffect([this](Function<void()> original) {
                                     ObjectIDSwap swap(this, 99);
                                     original();
                                 })
                                 .build()) {}

    void postPlayLayerInit() override;

    static void setPlayerSize(PlayerObject* player, float s);

    static void runScaleAction(PlayerObject* player);

    static void runGrowToBigEffects(PlayerObject* player);

    static void runShrinkToRegularEffects(PlayerObject* player);
};
