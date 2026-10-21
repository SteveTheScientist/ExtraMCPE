#pragma once
#include "commands/Command.hpp"
#include <string>
#include <vector>

class CommandGive : public Command {
public:
    CommandGive();

    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) override;
    virtual std::string help(Minecraft& mc) override;
};
