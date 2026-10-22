#pragma once
#include "commands/Command.hpp"
#include <string>
#include <vector>

struct ResolvedItem {
    int id;
    std::string matchedKey;
    std::string displayName;
    int maxStackSize;
};

std::string formatRegisteredName(const std::string& name);
std::string getDisplayName(int itemId, int auxVal, const std::string& matchedKey = "", const std::string& typedName = "");
bool resolveItem(const std::string& rawItemStr, ResolvedItem& outItem);

class CommandGive : public Command {
public:
    CommandGive();

    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) override;
    virtual std::string help(Minecraft& mc) override;
};
