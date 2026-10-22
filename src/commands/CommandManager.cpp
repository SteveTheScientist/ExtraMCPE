#include "CommandManager.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include "client/Minecraft.h"
#include "commands/Command.hpp"
#include "commands/CommandHelp.hpp"
#include "commands/CommandKick.hpp"
#include "commands/CommandOp.hpp"
#include "commands/CommandBan.hpp"
#include "commands/CommandGimmieItems.hpp"
#include "commands/CommandGive.hpp"
#include "commands/CommandGameMode.hpp"
#include "commands/CommandTime.hpp"
#include "commands/CommandSeed.hpp"
#include "commands/CommandList.hpp"
#include "commands/CommandTeleport.hpp"
#include "commands/CommandKill.hpp"
#include "commands/CommandClear.hpp"
#include "commands/CommandSpawnPoint.hpp"
#include "commands/CommandSetWorldSpawn.hpp"
#include "commands/CommandSay.hpp"
#include "commands/CommandMe.hpp"
#include "commands/CommandMsg.hpp"
#include "network/packet/ChatPacket.h"
#include "network/RakNetInstance.h"
#include "world/level/Level.h"

CommandManager::CommandManager() {
    registerAllCommands();
}

void CommandManager::registerAllCommands() {
    m_commands.push_back(new CommandHelp());
    m_commands.push_back(new CommandKick());
    m_commands.push_back(new CommandOp());
    m_commands.push_back(new CommandBan());
    m_commands.push_back(new CommandGive());
    m_commands.push_back(new CommandGameMode());
    m_commands.push_back(new CommandTime());
    m_commands.push_back(new CommandSeed());
    m_commands.push_back(new CommandList());
    m_commands.push_back(new CommandTeleport());
    m_commands.push_back(new CommandKill());
    m_commands.push_back(new CommandClear());
    m_commands.push_back(new CommandSpawnPoint());
    m_commands.push_back(new CommandSetWorldSpawn());
    m_commands.push_back(new CommandSay());
    m_commands.push_back(new CommandMe());
    m_commands.push_back(new CommandMsg("msg"));
    m_commands.push_back(new CommandMsg("tell"));
    m_commands.push_back(new CommandMsg("w"));
    // m_commands.push_back(new CommandGimmieItems());
}

std::vector<std::string> CommandManager::getListAllCommands() {
    std::vector<std::string> ret;

    for (auto& cmd : m_commands) {
        ret.push_back(cmd->getName());
    }

    return ret;
}

std::string CommandManager::execute(Minecraft& mc, Player& player, const std::string& input) {
    std::istringstream ss(input);
    std::string cmd;

    ss >> cmd;

    if (cmd.empty() || cmd == "?") {
        cmd = "help";
    }

    auto it = std::find_if(m_commands.begin(), m_commands.end(), [cmd](auto& it) -> bool {
        return it->getName() == cmd;
    });

    if (it == m_commands.end()) {
        return "\xc2\xa7" "cUnknown command. Try /help for a list of commands.";
    }

    std::vector<std::string> args;

    std::string tok;
    while (ss >> tok) args.push_back(tok);

    return (*it)->execute(mc, player, args);
}

Command* CommandManager::getCommand(const std::string& name) {
    std::string lookupName = name;
    if (lookupName == "?") lookupName = "help";

    auto it = std::find_if(m_commands.begin(), m_commands.end(), [lookupName](auto& it) -> bool {
        return it->getName() == lookupName;
    });

    if (it != m_commands.end()) {
        return *it;
    }

    return nullptr;
}
