#include "GameRules.hpp"
#include "nbt/CompoundTag.h"

#include <algorithm>
#include <sstream>
#include <cstdlib>

namespace {

static std::map<std::string, GameRules::RuleDefinition>& getRegistry() {
    static std::map<std::string, GameRules::RuleDefinition> s_registry;
    return s_registry;
}

static std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

static bool parseBool(const std::string& str, bool& outVal) {
    std::string lower = toLower(str);
    if (lower == "true" || lower == "1") {
        outVal = true;
        return true;
    }
    if (lower == "false" || lower == "0") {
        outVal = false;
        return true;
    }
    return false;
}

static bool parseInt(const std::string& str, int& outVal) {
    if (str.empty()) return false;
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;
    if (start >= str.length()) return false;
    for (size_t i = start; i < str.length(); ++i) {
        if (str[i] < '0' || str[i] > '9') return false;
    }
    outVal = std::atoi(str.c_str());
    return true;
}

static std::string intToString(int val) {
    std::ostringstream ss;
    ss << val;
    return ss.str();
}

} // namespace

void GameRules::registerRule(const std::string& name, Type type, const std::string& defaultValue) {
    RuleDefinition def;
    def.name = name;
    def.type = type;
    def.defaultValue = defaultValue;
    getRegistry()[name] = def;
}

void GameRules::registerRule(const std::string& name, bool defaultValue) {
    registerRule(name, TYPE_BOOL, defaultValue ? "true" : "false");
}

void GameRules::registerRule(const std::string& name, int defaultValue) {
    registerRule(name, TYPE_INT, intToString(defaultValue));
}

bool GameRules::isRuleRegistered(const std::string& name) {
    const auto& reg = getRegistry();
    return reg.find(name) != reg.end();
}

bool GameRules::hasRegisteredRules() {
    return !getRegistry().empty();
}

GameRules::Type GameRules::getRuleType(const std::string& name) {
    const auto& reg = getRegistry();
    auto it = reg.find(name);
    if (it != reg.end()) {
        return it->second.type;
    }
    return TYPE_BOOL;
}

std::string GameRules::getRuleDefaultValue(const std::string& name) {
    const auto& reg = getRegistry();
    auto it = reg.find(name);
    if (it != reg.end()) {
        return it->second.defaultValue;
    }
    return "";
}

std::vector<std::string> GameRules::getRegisteredRuleNames() {
    std::vector<std::string> names;
    const auto& reg = getRegistry();
    for (auto it = reg.begin(); it != reg.end(); ++it) {
        names.push_back(it->first);
    }
    return names;
}

GameRules::GameRules() {
}

bool GameRules::getBool(const std::string& name) const {
    std::string valStr = getString(name);
    bool boolVal = false;
    if (parseBool(valStr, boolVal)) {
        return boolVal;
    }
    return false;
}

int GameRules::getInt(const std::string& name) const {
    std::string valStr = getString(name);
    int intVal = 0;
    if (parseInt(valStr, intVal)) {
        return intVal;
    }
    return 0;
}

std::string GameRules::getString(const std::string& name) const {
    auto it = m_values.find(name);
    if (it != m_values.end()) {
        return it->second;
    }
    return getRuleDefaultValue(name);
}

bool GameRules::set(const std::string& name, const std::string& valueStr) {
    if (!isRuleRegistered(name)) {
        return false;
    }
    Type type = getRuleType(name);
    if (type == TYPE_BOOL) {
        bool boolVal = false;
        if (!parseBool(valueStr, boolVal)) {
            return false;
        }
        m_values[name] = boolVal ? "true" : "false";
        return true;
    } else if (type == TYPE_INT) {
        int intVal = 0;
        if (!parseInt(valueStr, intVal)) {
            return false;
        }
        m_values[name] = intToString(intVal);
        return true;
    }
    return false;
}

bool GameRules::set(const std::string& name, bool val) {
    return set(name, val ? "true" : "false");
}

bool GameRules::set(const std::string& name, int val) {
    return set(name, intToString(val));
}

void GameRules::write(CompoundTag* tag) const {
    if (!tag) return;
    for (auto it = m_values.begin(); it != m_values.end(); ++it) {
        tag->putString(it->first, it->second);
    }
}

void GameRules::read(const CompoundTag* tag) {
    if (!tag) return;
    m_values.clear();
    const auto& reg = getRegistry();
    for (auto it = reg.begin(); it != reg.end(); ++it) {
        const std::string& ruleName = it->first;
        if (tag->contains(ruleName, Tag::TAG_String)) {
            std::string valStr = tag->getString(ruleName);
            set(ruleName, valStr);
        }
    }
}
