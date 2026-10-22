#pragma once
#include "commands/Command.hpp"

class CommandList : public Command {
public:
    CommandList();

    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) override;
    virtual std::string help(Minecraft& mc) override;
};
