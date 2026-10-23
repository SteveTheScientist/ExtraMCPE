#include "CommandGameRule.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/level/GameRules.hpp"

#include <algorithm>
#include <sstream>

CommandGameRule::CommandGameRule() : Command("gamerule") {
    m_syntax.push_back(CommandArgument("rule", CommandArgType::Enum, true, GameRules::getRegisteredRuleNames()));
    m_syntax.push_back(CommandArgument("value", CommandArgType::Text, true));
}

const std::vector<CommandArgument>& CommandGameRule::getSyntax() const {
    if (!m_syntax.empty()) {
        const_cast<CommandGameRule*>(this)->m_syntax[0].allowedValues = GameRules::getRegisteredRuleNames();
    }
    return m_syntax;
}

std::string CommandGameRule::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (!isPlayerOp(mc, player)) {
        return SECTION "cYou do not have permission to use this command";
    }

    if (!GameRules::hasRegisteredRules()) {
        return "There are no game rules yet";
    }

    if (args.empty()) {
        return SECTION "c" + help(mc);
    }

    std::string ruleName = args[0];

    if (!GameRules::isRuleRegistered(ruleName)) {
        return SECTION "cUnknown game rule: " + ruleName;
    }

    if (mc.level == NULL || mc.level->getLevelData() == NULL) {
        return SECTION "cNo world is loaded";
    }

    GameRules& rules = mc.level->getLevelData()->getGameRules();

    if (args.size() == 1) {
        std::string curVal = rules.getString(ruleName);
        return "Game rule " + ruleName + " is currently set to: " + curVal;
    }

    std::string newVal = args[1];
    if (!rules.set(ruleName, newVal)) {
        return SECTION "c" + help(mc);
    }

    std::string updatedVal = rules.getString(ruleName);
    return "Game rule " + ruleName + " has been updated to " + updatedVal;
}

std::string CommandGameRule::help(Minecraft& mc) {
    return "Usage: /gamerule <rule> [value]";
}
