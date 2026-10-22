#pragma once
#include <string>
#include <vector>

#ifndef SECTION
#define SECTION "\xc2\xa7"
#endif

class Minecraft;
class Player;

enum class CommandArgType {
    Player,
    Item,
    Number,
    Integer,
    Enum,
    Coordinate,
    Text
};

struct CommandArgument {
    std::string name;
    CommandArgType type;
    bool optional;
    std::vector<std::string> allowedValues;

    CommandArgument(const std::string& name, CommandArgType type, bool optional = false, const std::vector<std::string>& allowedValues = {})
        : name(name), type(type), optional(optional), allowedValues(allowedValues) {}
};

class Command {
public:
    const std::string& getName() const { return m_name; }

    bool isPlayerOp(Minecraft& mc, Player& player);
    virtual bool isOperatorCommand() const { return true; }

    virtual const std::vector<CommandArgument>& getSyntax() const { return m_syntax; }

    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) = 0;
    virtual std::string help(Minecraft& mc) = 0;

protected:
    Command(const std::string& name) : m_name(name) {}

    const std::string m_name;
    std::vector<CommandArgument> m_syntax;
};
