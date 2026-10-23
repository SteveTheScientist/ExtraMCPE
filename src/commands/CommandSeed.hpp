#pragma once
#include "commands/Command.hpp"

class CommandSeed : public Command {
public:
    CommandSeed();

    virtual bool isOperatorCommand() const override { return false; }
    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) override;
    virtual std::string help(Minecraft& mc) override;
};
