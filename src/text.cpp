#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <regex>

$execute{ GEODE_WINDOWS(SetConsoleOutputCP(65001)); }; //utf-8 console

//data values
inline static matjson::Value replaces;