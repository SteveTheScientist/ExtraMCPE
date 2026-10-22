#include "CommandSay.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/entity/player/Player.h"
#include "network/packet/ChatPacket.h"
#include "network/RakNetInstance.h"

#include <sstream>

CommandSay::CommandSay() : Command("say") {
    m_syntax.push_back(CommandArgument("text", CommandArgType::Text, false));
}

std::string CommandSay::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (!isPlayerOp(mc, player)) {
        return "\xc2\xa7" "cYou do not have permission to use this command";
    }

    if (args.empty()) {
        return help(mc);
    }

    std::string text = "";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) text += " ";
        text += args[i];
    }

    std::string broadcastMsg = "[" + player.name + "] " + text;

    mc.addMessage(broadcastMsg);

    if (mc.raknetInstance != NULL) {
        ChatPacket packet(broadcastMsg, false);
        mc.raknetInstance->send(packet);
    }

    return "";
}

std::string CommandSay::help(Minecraft& mc) {
    return "Usage: /say <text>";
}
