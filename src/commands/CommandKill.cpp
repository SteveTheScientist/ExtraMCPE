#include "CommandKill.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/entity/player/Player.h"

#include <algorithm>
#include <sstream>

namespace {

std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

} // namespace

CommandKill::CommandKill() : Command("kill") {}

std::string CommandKill::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (!isPlayerOp(mc, player)) {
        return "\xc2\xa7" "cYou do not have permission to use this command";
    }

    Player* targetPlayer = &player;

    if (!args.empty()) {
        std::string targetNameArg = args[0];
        if (targetNameArg == "@s" || targetNameArg == "@p") {
            targetPlayer = &player;
        } else {
            targetPlayer = NULL;
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
            if (targetPlayer == NULL) {
                return "Player not found: " + targetNameArg;
            }
        }
    }

    targetPlayer->health = 0;
    targetPlayer->die(NULL);

    return "Killed " + targetPlayer->name;
}

std::string CommandKill::help(Minecraft& mc) {
    return "Usage: /kill [player]";
}
