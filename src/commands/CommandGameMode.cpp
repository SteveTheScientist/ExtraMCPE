#include "CommandGameMode.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/level/LevelSettings.h"
#include "world/entity/player/Player.h"
#include "world/entity/player/Inventory.h"

#include <algorithm>
#include <sstream>

namespace {

std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

} // namespace

CommandGameMode::CommandGameMode() : Command("gamemode") {}

std::string CommandGameMode::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (!isPlayerOp(mc, player)) {
        return "\xc2\xa7" "cYou do not have permission to use this command";
    }

    if (args.empty()) {
        return help(mc);
    }

    std::string modeArg = toLower(args[0]);
    int newGameType = GameType::Undefined;
    std::string modeName = "";

    if (modeArg == "survival" || modeArg == "s" || modeArg == "0") {
        newGameType = GameType::Survival;
        modeName = "Survival Mode";
    } else if (modeArg == "creative" || modeArg == "c" || modeArg == "1") {
        newGameType = GameType::Creative;
        modeName = "Creative Mode";
    } else {
        return help(mc);
    }

    Player* targetPlayer = &player;

    if (args.size() >= 2) {
        std::string targetNameArg = args[1];
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

    if (mc.level != NULL && mc.level->getLevelData() != NULL) {
        mc.level->getLevelData()->setGameType(newGameType);
    }

    if (targetPlayer == &player) {
        return "Set own game mode to " + modeName;
    } else {
        return "Set " + targetPlayer->name + "'s game mode to " + modeName;
    }
}

std::string CommandGameMode::help(Minecraft& mc) {
    return "Usage: /gamemode <survival|creative|s|c|0|1> [player]";
}
