#include "CommandMe.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/entity/player/Player.h"
#include "network/packet/ChatPacket.h"
#include "network/RakNetInstance.h"

#include <sstream>

CommandMe::CommandMe() : Command("me") {}

std::string CommandMe::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (args.empty()) {
        return help(mc);
    }

    std::string action = "";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) action += " ";
        action += args[i];
    }

    std::string broadcastMsg = "* " + player.name + " " + action;

    mc.addMessage(broadcastMsg);

    if (mc.raknetInstance != NULL) {
        ChatPacket packet(broadcastMsg, false);
        mc.raknetInstance->send(packet);
    }

    return "";
}

std::string CommandMe::help(Minecraft& mc) {
    return "Usage: /me <action>";
}
