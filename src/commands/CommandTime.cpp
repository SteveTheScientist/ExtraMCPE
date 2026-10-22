#include "CommandTime.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"

#include <algorithm>
#include <sstream>
#include <cstdlib>

namespace {

std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

} // namespace

CommandTime::CommandTime() : Command("time") {
    m_syntax.push_back(CommandArgument("action", CommandArgType::Enum, false, {"set", "add"}));
    m_syntax.push_back(CommandArgument("value", CommandArgType::Enum, false, {"day", "noon", "night", "midnight"}));
}

std::string CommandTime::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (!isPlayerOp(mc, player)) {
        return "\xc2\xa7" "cYou do not have permission to use this command";
    }

    if (args.size() < 2) {
        return help(mc);
    }

    std::string subCmd = toLower(args[0]);
    std::string valArg = toLower(args[1]);

    if (subCmd == "set") {
        long targetTicks = 0;
        if (valArg == "day") {
            targetTicks = 1000;
        } else if (valArg == "noon") {
            targetTicks = 6000;
        } else if (valArg == "night") {
            targetTicks = 13000;
        } else if (valArg == "midnight") {
            targetTicks = 18000;
        } else {
            targetTicks = std::atol(valArg.c_str());
        }

        if (mc.level != NULL) {
            mc.level->setTime(targetTicks);
        }

        std::ostringstream ss;
        ss << "Set the time to " << targetTicks;
        return ss.str();
    } else if (subCmd == "add") {
        long addTicks = std::atol(valArg.c_str());

        if (mc.level != NULL) {
            long currentTicks = mc.level->getTime();
            mc.level->setTime(currentTicks + addTicks);
        }

        std::ostringstream ss;
        ss << "Added " << addTicks << " to the time";
        return ss.str();
    }

    return help(mc);
}

std::string CommandTime::help(Minecraft& mc) {
    return "Usage: /time set <day|night|noon|midnight|ticks> or /time add <ticks>";
}
