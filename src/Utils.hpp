#pragma once

#include <Geode/utils/string.hpp>

template <geode::utils::string::ConstexprString S, typename T>
T const& getSettingFast() {
    static T value = (
        geode::listenForSettingChanges<T>(S.data(), [](T val) {
            value = std::move(val);
        }),
        geode::getMod()->getSettingValue<T>(S.data())
    );
    return value;
}

namespace geode::log {
    template <typename... Args>
    void dev(format::FmtStr<Args...> str, Args&&... args) {
        if (getSettingFast<"dev-logging", bool>()) log::debug(str, std::forward<Args>(args)...);
    }
}

bool isAprilFools();