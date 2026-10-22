#include "CommandMsg.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/entity/player/Player.h"
#include "network/packet/ChatPacket.h"
#include "network/RakNetInstance.h"

#include <algorithm>
#include <sstream>

namespace {

std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

} // namespace

CommandMsg::CommandMsg(const std::string& name) : Command(name) {
    m_syntax.push_back(CommandArgument("player", CommandArgType::Player, false));
    m_syntax.push_back(CommandArgument("text", CommandArgType::Text, false));
}

std::string CommandMsg::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return help(mc);
    }

    std::string targetNameArg = args[0];
    Player* targetPlayer = NULL;

    if (targetNameArg == "@s" || targetNameArg == "@p") {
        targetPlayer = &player;
    } else {
        std::string targetLower = toLower(targetNameArg);
        if (mc.level != NULL) {
            for (auto p : mc.level->players) {
                if (p != NULL) {
                    std::string pLower = toLower(p->name);
                    if (pLower == targetLower) {
                        targetPlayer = p;
                        break;
                    }
                }
            }
        }
    }

    if (targetPlayer == NULL) {
        return "Player not found: " + targetNameArg;
    }

    std::string text = "";
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) text += " ";
        text += args[i];
    }

    // Format in gray italic: \xc2\xa7o (italic) \xc2\xa77 (gray) ... \xc2\xa7r (reset)
    std::string senderMsg = "\xc2\xa7o\xc2\xa7" "7You whisper to " + targetPlayer->name + ": " + text + "\xc2\xa7r";
    std::string targetMsg = "\xc2\xa7o\xc2\xa7" "7" + player.name + " whispers to you: " + text + "\xc2\xa7r";

    if (targetPlayer == &player) {
        return senderMsg;
    }

    if (mc.raknetInstance != NULL) {
        ChatPacket packet(targetMsg, false);
        mc.raknetInstance->send(targetPlayer->owner, packet);
    }

    return senderMsg;
}

std::string CommandMsg::help(Minecraft& mc) {
    return "Usage: /" + m_name + " <player> <text>";
}
