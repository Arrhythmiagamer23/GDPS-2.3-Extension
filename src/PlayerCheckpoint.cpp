#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/PlayerCheckpoint.hpp>
class $modify(MyPlayerCheckpoint, PlayerCheckpoint) {
    // important!!! keep me!!!
    // used by PlayerObject::saveToCheckpoint and loadFromCheckpoint
    struct Fields {
        float m_vehicleSize = 1.f;
    };
};
