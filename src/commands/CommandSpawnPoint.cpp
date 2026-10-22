#include "CommandSpawnPoint.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/entity/player/Player.h"

#include <algorithm>
#include <sstream>
#include <cstdlib>

namespace {

std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

bool parseCoord(const std::string& arg, float baseVal, int& outVal) {
    if (arg.empty()) return false;
    if (arg[0] == '~') {
        if (arg.length() == 1) {
            outVal = (int)baseVal;
            return true;
        }
        outVal = (int)(baseVal + (float)std::atof(arg.substr(1).c_str()));
        return true;
    }
    outVal = std::atoi(arg.c_str());
    return true;
}

} // namespace

CommandSpawnPoint::CommandSpawnPoint() : Command("spawnpoint") {
    m_syntax.push_back(CommandArgument("player", CommandArgType::Player, true));
    m_syntax.push_back(CommandArgument("x", CommandArgType::Coordinate, true));
    m_syntax.push_back(CommandArgument("y", CommandArgType::Coordinate, true));
    m_syntax.push_back(CommandArgument("z", CommandArgType::Coordinate, true));
}

std::string CommandSpawnPoint::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (!isPlayerOp(mc, player)) {
        return SECTION "cYou do not have permission to use this command";
    }

    Player* targetPlayer = &player;
    int spawnX = (int)player.x;
    int spawnY = (int)player.y;
    int spawnZ = (int)player.z;

    if (args.size() == 1) {
        // /spawnpoint [player]
        std::string nameArg = args[0];
        if (nameArg == "@s" || nameArg == "@p") {
            targetPlayer = &player;
        } else {
            targetPlayer = NULL;
            std::string lowerArg = toLower(nameArg);
            if (mc.level != NULL) {
                for (auto p : mc.level->players) {
                    if (p != NULL) {
                        std::string pLower = toLower(p->name);
                        if (pLower == lowerArg) {
                            targetPlayer = p;
                            break;
                        }
                    }
                }
            }
            if (targetPlayer == NULL) {
                return "Player not found: " + nameArg;
            }
        }
        spawnX = (int)targetPlayer->x;
        spawnY = (int)targetPlayer->y;
        spawnZ = (int)targetPlayer->z;
    } else if (args.size() == 3) {
        // /spawnpoint <x> <y> <z>
        targetPlayer = &player;
        if (!parseCoord(args[0], targetPlayer->x, spawnX) ||
            !parseCoord(args[1], targetPlayer->y, spawnY) ||
            !parseCoord(args[2], targetPlayer->z, spawnZ)) {
            return help(mc);
        }
    } else if (args.size() >= 4) {
        // /spawnpoint <player> <x> <y> <z>
        std::string nameArg = args[0];
        if (nameArg == "@s" || nameArg == "@p") {
            targetPlayer = &player;
        } else {
            targetPlayer = NULL;
            std::string lowerArg = toLower(nameArg);
            if (mc.level != NULL) {
                for (auto p : mc.level->players) {
                    if (p != NULL) {
                        std::string pLower = toLower(p->name);
                        if (pLower == lowerArg) {
                            targetPlayer = p;
                            break;
                        }
                    }
                }
            }
            if (targetPlayer == NULL) {
                return "Player not found: " + nameArg;
            }
        }
        if (!parseCoord(args[1], targetPlayer->x, spawnX) ||
            !parseCoord(args[2], targetPlayer->y, spawnY) ||
            !parseCoord(args[3], targetPlayer->z, spawnZ)) {
            return help(mc);
        }
    }

    targetPlayer->setRespawnPosition(Pos(spawnX, spawnY, spawnZ));

    std::ostringstream ss;
    ss << "Set spawn point to " << spawnX << ", " << spawnY << ", " << spawnZ;
    return ss.str();
}

std::string CommandSpawnPoint::help(Minecraft& mc) {
    return "Usage: /spawnpoint [player] [x y z]";
}
