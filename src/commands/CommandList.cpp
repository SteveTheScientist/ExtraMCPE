#include "CommandList.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/entity/player/Player.h"

#include <sstream>

CommandList::CommandList() : Command("list") {}

std::string CommandList::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (!isPlayerOp(mc, player)) {
        return "\xc2\xa7" "cYou do not have permission to use this command";
    }

    std::vector<Player*> activePlayers;
    if (mc.level != NULL) {
        for (auto p : mc.level->players) {
            if (p != NULL) {
                activePlayers.push_back(p);
            }
        }
    }

    std::ostringstream ss;
    ss << "There are " << activePlayers.size() << " players online:";
    for (size_t i = 0; i < activePlayers.size(); ++i) {
        if (i == 0) {
            ss << " " << activePlayers[i]->name;
        } else {
            ss << ", " << activePlayers[i]->name;
        }
    }

    return ss.str();
}

std::string CommandList::help(Minecraft& mc) {
    return "Usage: /list";
}
