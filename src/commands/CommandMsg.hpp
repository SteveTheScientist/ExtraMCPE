#pragma once
#include "commands/Command.hpp"

class CommandMsg : public Command {
public:
    CommandMsg(const std::string& name = "msg");

    virtual bool isOperatorCommand() const override { return false; }

    virtual std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) override;
    virtual std::string help(Minecraft& mc) override;
};
