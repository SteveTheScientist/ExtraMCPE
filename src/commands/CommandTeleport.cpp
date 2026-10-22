#include "CommandTeleport.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/entity/player/Player.h"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstdlib>

namespace {

std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

Player* findPlayer(Minecraft& mc, const std::string& nameArg, Player& executingPlayer) {
    if (nameArg == "@s" || nameArg == "@p") {
        return &executingPlayer;
    }
    std::string lowerArg = toLower(nameArg);
    if (mc.level != NULL) {
        for (auto p : mc.level->players) {
            if (p != NULL) {
                std::string pLower = toLower(p->name);
                if (pLower == lowerArg) {
                    return p;
                }
            }
        }
    }
    return NULL;
}

bool parseCoord(const std::string& arg, float baseVal, float& outVal) {
    if (arg.empty()) return false;
    if (arg[0] == '~') {
        if (arg.length() == 1) {
            outVal = baseVal;
            return true;
        }
        outVal = baseVal + (float)std::atof(arg.substr(1).c_str());
        return true;
    }
    outVal = (float)std::atof(arg.c_str());
    return true;
}

} // namespace

CommandTeleport::CommandTeleport() : Command("tp") {
    m_syntax.push_back(CommandArgument("player", CommandArgType::Player, false));
    m_syntax.push_back(CommandArgument("target", CommandArgType::Player, true));
}

std::string CommandTeleport::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (!isPlayerOp(mc, player)) {
        return "\xc2\xa7" "cYou do not have permission to use this command";
    }

    if (args.empty()) {
        return help(mc);
    }

    Player* targetToMove = NULL;
    float destX = 0.0f, destY = 0.0f, destZ = 0.0f;

    if (args.size() == 1) {
        // /tp <target player> (teleport self to target player)
        Player* destPlayer = findPlayer(mc, args[0], player);
        if (destPlayer == NULL) {
            return "Player not found: " + args[0];
        }
        targetToMove = &player;
        destX = destPlayer->x;
        destY = destPlayer->y;
        destZ = destPlayer->z;
    } else if (args.size() == 2) {
        // /tp <player> <target player> (teleport player to target player)
        targetToMove = findPlayer(mc, args[0], player);
        if (targetToMove == NULL) {
            return "Player not found: " + args[0];
        }
        Player* destPlayer = findPlayer(mc, args[1], player);
        if (destPlayer == NULL) {
            return "Player not found: " + args[1];
        }
        destX = destPlayer->x;
        destY = destPlayer->y;
        destZ = destPlayer->z;
    } else if (args.size() == 3) {
        // /tp <x> <y> <z> (teleport self to coords)
        targetToMove = &player;
        if (!parseCoord(args[0], targetToMove->x, destX) ||
            !parseCoord(args[1], targetToMove->y, destY) ||
            !parseCoord(args[2], targetToMove->z, destZ)) {
            return help(mc);
        }
    } else if (args.size() >= 4) {
        // /tp <player> <x> <y> <z>
        targetToMove = findPlayer(mc, args[0], player);
        if (targetToMove == NULL) {
            return "Player not found: " + args[0];
        }
        if (!parseCoord(args[1], targetToMove->x, destX) ||
            !parseCoord(args[2], targetToMove->y, destY) ||
            !parseCoord(args[3], targetToMove->z, destZ)) {
            return help(mc);
        }
    }

    // Clamp Y to world height [0, 128]
    if (destY < 0.0f) destY = 0.0f;
    if (destY > 128.0f) destY = 128.0f;

    if (mc.level != NULL) {
        mc.level->getChunkAt((int)destX, (int)destZ);
    }

    targetToMove->moveTo(destX, destY, destZ, targetToMove->yRot, targetToMove->xRot);
    targetToMove->setPos(destX, destY, destZ);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "Teleported " << targetToMove->name << " to " << destX << ", " << destY << ", " << destZ;
    return ss.str();
}

std::string CommandTeleport::help(Minecraft& mc) {
    return "Usage: /tp [player] <target player> or /tp [player] <x> <y> <z>";
}
