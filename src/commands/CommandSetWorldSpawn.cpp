#include "CommandSetWorldSpawn.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/entity/player/Player.h"

#include <sstream>
#include <cstdlib>

namespace {

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

CommandSetWorldSpawn::CommandSetWorldSpawn() : Command("setworldspawn") {
    m_syntax.push_back(CommandArgument("x", CommandArgType::Coordinate, true));
    m_syntax.push_back(CommandArgument("y", CommandArgType::Coordinate, true));
    m_syntax.push_back(CommandArgument("z", CommandArgType::Coordinate, true));
}

std::string CommandSetWorldSpawn::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (!isPlayerOp(mc, player)) {
        return "\xc2\xa7" "cYou do not have permission to use this command";
    }

    int spawnX = (int)player.x;
    int spawnY = (int)player.y;
    int spawnZ = (int)player.z;

    if (args.size() >= 3) {
        if (!parseCoord(args[0], player.x, spawnX) ||
            !parseCoord(args[1], player.y, spawnY) ||
            !parseCoord(args[2], player.z, spawnZ)) {
            return help(mc);
        }
    }

    if (mc.level != NULL) {
        mc.level->setSpawnPos(Pos(spawnX, spawnY, spawnZ));
    }

    std::ostringstream ss;
    ss << "Set the world spawn point to " << spawnX << ", " << spawnY << ", " << spawnZ;
    return ss.str();
}

std::string CommandSetWorldSpawn::help(Minecraft& mc) {
    return "Usage: /setworldspawn [x y z]";
}
