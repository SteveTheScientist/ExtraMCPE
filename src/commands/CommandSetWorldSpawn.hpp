#pragma once
#include "commands/Command.hpp"

class CommandSetWorldSpawn : public Command {
public:
    CommandSetWorldSpawn();

    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) override;
    virtual std::string help(Minecraft& mc) override;
};
