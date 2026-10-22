#pragma once
#include "commands/Command.hpp"

class CommandKill : public Command {
public:
    CommandKill();

    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) override;
    virtual std::string help(Minecraft& mc) override;
};
