#include "Command.hpp"

class CommandHelp : public Command {
public:
    CommandHelp();

    virtual bool isOperatorCommand() const override { return false; }

    std::string execute(Minecraft& mc, Player& player, const std::vector<std::string>& args);
    std::string help(Minecraft& mc);
};