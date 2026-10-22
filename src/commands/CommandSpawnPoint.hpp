#pragma once
#include "commands/Command.hpp"

class CommandSpawnPoint : public Command {
public:
    CommandSpawnPoint();

    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) override;
    virtual std::string help(Minecraft& mc) override;
};
