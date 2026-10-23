#include "CommandSeed.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"

#include <sstream>

CommandSeed::CommandSeed() : Command("seed") {}

std::string CommandSeed::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    long seed = 0;
    if (mc.level != NULL) {
        seed = mc.level->getSeed();
    }

    std::ostringstream ss;
    ss << "Seed: " << seed;
    return ss.str();
}

std::string CommandSeed::help(Minecraft& mc) {
    return "Usage: /seed";
}
