#include "Utils.hpp"

using namespace geode::prelude;

bool isAprilFools() {
    auto now = std::chrono::system_clock::now();
    auto timeNow = std::chrono::system_clock::to_time_t(now);
    auto tm_local = asp::localtime(timeNow);

    return tm_local.tm_mon == 3 && tm_local.tm_mday == 1;
}