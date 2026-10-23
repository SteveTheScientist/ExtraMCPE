#pragma once

#include <string>
#include <vector>
#include <map>

class CompoundTag;

class GameRules {
public:
    enum Type {
        TYPE_BOOL = 0,
        TYPE_INT = 1
    };

    struct RuleDefinition {
        std::string name;
        Type type;
        std::string defaultValue;
    };

    // Global rule registry
    static void registerRule(const std::string& name, Type type, const std::string& defaultValue);
    static void registerRule(const std::string& name, bool defaultValue);
    static void registerRule(const std::string& name, int defaultValue);

    static bool isRuleRegistered(const std::string& name);
    static bool hasRegisteredRules();
    static Type getRuleType(const std::string& name);
    static std::string getRuleDefaultValue(const std::string& name);
    static std::vector<std::string> getRegisteredRuleNames();

    GameRules();

    // Instance rule value access
    bool getBool(const std::string& name) const;
    int getInt(const std::string& name) const;
    std::string getString(const std::string& name) const;

    bool set(const std::string& name, const std::string& valueStr);
    bool set(const std::string& name, bool val);
    bool set(const std::string& name, int val);

    // Read/write NBT
    void write(CompoundTag* tag) const;
    void read(const CompoundTag* tag);

private:
    std::map<std::string, std::string> m_values;
};
