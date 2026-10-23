#pragma once
#include "commands/Command.hpp"

class CommandDifficulty : public Command {
public:
    CommandDifficulty();

    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) override;
    virtual std::string help(Minecraft& mc) override;
};
