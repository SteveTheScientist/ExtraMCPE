#include "CommandClear.hpp"
#include "CommandGive.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/entity/player/Player.h"
#include "world/entity/player/Inventory.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/level/tile/Tile.h"

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

CommandClear::CommandClear() : Command("clear") {
    m_syntax.push_back(CommandArgument("player", CommandArgType::Player, true));
    m_syntax.push_back(CommandArgument("item", CommandArgType::Item, true));
    m_syntax.push_back(CommandArgument("maxCount", CommandArgType::Integer, true));
}

std::string CommandClear::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
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

    int filterItemId = -1;
    if (args.size() >= 2) {
        ResolvedItem resItem;
        if (!resolveItem(args[1], resItem)) {
            return "Unknown item: " + args[1];
        }
        filterItemId = resItem.id;
    }

    int maxCount = -1;
    if (args.size() >= 3) {
        maxCount = std::atoi(args[2].c_str());
    }

    int removedTotal = 0;
    Inventory* inv = targetPlayer->inventory;
    if (inv != NULL) {
        int containerSize = inv->getContainerSize();
        for (int i = 0; i < containerSize; ++i) {
            ItemInstance* item = inv->getItem(i);
            if (item != NULL && !item->isNull() && item->count > 0) {
                if (filterItemId == -1 || item->id == filterItemId) {
                    if (maxCount > 0) {
                        int remainingToClear = maxCount - removedTotal;
                        if (remainingToClear <= 0) {
                            break;
                        }
                        int toRemove = std::min(item->count, remainingToClear);
                        inv->FillingContainer::removeItem(i, toRemove);
                        removedTotal += toRemove;
                    } else {
                        removedTotal += item->count;
                        inv->clearSlot(i);
                    }
                }
            }
        }
    }

    if (removedTotal > 0) {
        std::ostringstream ss;
        ss << "Removed " << removedTotal << " items from " << targetPlayer->name;
        return ss.str();
    } else {
        return "No items were found on player " + targetPlayer->name;
    }
}

std::string CommandClear::help(Minecraft& mc) {
    return "Usage: /clear [player] [item] [maxCount]";
}
