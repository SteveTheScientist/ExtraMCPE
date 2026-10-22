#pragma once
#include "commands/Command.hpp"

class CommandMe : public Command {
public:
    CommandMe();

    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) override;
    virtual std::string help(Minecraft& mc) override;
};
