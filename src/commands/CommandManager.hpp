#pragma once
#include <string>
#include <vector>

#include "Command.hpp"

class CommandManager {
public:
    CommandManager();

    std::vector<std::string> getListAllCommands();
    std::vector<std::string> getListAllCommands(Minecraft& mc);

    std::string execute(Minecraft& mc, Player& player, const std::string& input);

    Command* getCommand(const std::string& name);
    const std::vector<Command*>& getAllCommands() const { return m_commands; }

private:
    void registerAllCommands();
    
    std::vector<Command*> m_commands;
};