#pragma once
#include "commands/Command.hpp"

class CommandSeed : public Command {
public:
    CommandSeed();

    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) override;
    virtual std::string help(Minecraft& mc) override;
};
