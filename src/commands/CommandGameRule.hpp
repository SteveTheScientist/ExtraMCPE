#pragma once
#include "commands/Command.hpp"

class CommandGameRule : public Command {
public:
    CommandGameRule();

    virtual const std::vector<CommandArgument>& getSyntax() const override;
    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) override;
    virtual std::string help(Minecraft& mc) override;
};
