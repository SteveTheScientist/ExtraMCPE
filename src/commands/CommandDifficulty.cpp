#include "CommandDifficulty.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/Difficulty.h"

#include <algorithm>
#include <sstream>

namespace {

std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

std::string getDifficultyName(int diff) {
    switch (diff) {
        case Difficulty::PEACEFUL: return "Peaceful";
        case Difficulty::EASY: return "Easy";
        case Difficulty::NORMAL: return "Normal";
        case Difficulty::HARD: return "Hard";
        default: return "Normal";
    }
}

} // namespace

CommandDifficulty::CommandDifficulty() : Command("difficulty") {
    m_syntax.push_back(CommandArgument("level", CommandArgType::Enum, true, {"peaceful", "easy", "normal", "hard", "0", "1", "2", "3"}));
}

std::string CommandDifficulty::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (!isPlayerOp(mc, player)) {
        return SECTION "cYou do not have permission to use this command";
    }

    if (args.empty()) {
        int curDiff = mc.options.getIntValue(OPTIONS_DIFFICULTY);
        if (mc.level != NULL) {
            curDiff = mc.level->difficulty;
        }
        return "The difficulty is " + getDifficultyName(curDiff);
    }

    std::string diffArg = toLower(args[0]);
    int newDiff = -1;
    std::string diffName = "";

    if (diffArg == "peaceful" || diffArg == "p" || diffArg == "0") {
        newDiff = Difficulty::PEACEFUL;
        diffName = "Peaceful";
    } else if (diffArg == "easy" || diffArg == "e" || diffArg == "1") {
        newDiff = Difficulty::EASY;
        diffName = "Easy";
    } else if (diffArg == "normal" || diffArg == "n" || diffArg == "2") {
        newDiff = Difficulty::NORMAL;
        diffName = "Normal";
    } else if (diffArg == "hard" || diffArg == "h" || diffArg == "3") {
        newDiff = Difficulty::HARD;
        diffName = "Hard";
    } else {
        return SECTION "c" + help(mc);
    }

    mc.options.set(OPTIONS_DIFFICULTY, newDiff);
    mc.options.save();

    if (mc.level != NULL) {
        mc.level->difficulty = newDiff;
    }

    return "Set game difficulty to " + diffName;
}

std::string CommandDifficulty::help(Minecraft& mc) {
    return "Usage: /difficulty <peaceful|easy|normal|hard|0|1|2|3>";
}
