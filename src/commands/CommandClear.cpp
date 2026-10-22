#include "CommandClear.hpp"
#include "client/Minecraft.h"
#include "world/level/Level.h"
#include "world/entity/player/Player.h"
#include "world/entity/player/Inventory.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/level/tile/Tile.h"

#include <algorithm>
#include <sstream>
#include <map>
#include <cstdlib>

namespace {

struct ResolvedItem {
    int id;
};

std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

std::string getRawDescId(const std::string& descId) {
    std::string cleaned = descId;
    if (cleaned.rfind("tile.", 0) == 0) {
        cleaned = cleaned.substr(5);
    } else if (cleaned.rfind("item.", 0) == 0) {
        cleaned = cleaned.substr(5);
    }
    size_t dotPos = cleaned.find(".name");
    if (dotPos != std::string::npos) {
        cleaned = cleaned.substr(0, dotPos);
    }
    return toLower(cleaned);
}

std::string cleanDescriptionId(const std::string& descId) {
    std::string cleaned = descId;
    if (cleaned.rfind("tile.", 0) == 0) {
        cleaned = cleaned.substr(5);
    } else if (cleaned.rfind("item.", 0) == 0) {
        cleaned = cleaned.substr(5);
    }
    size_t dotPos = cleaned.find(".name");
    if (dotPos != std::string::npos) {
        cleaned = cleaned.substr(0, dotPos);
    }

    std::string result = "";
    for (size_t i = 0; i < cleaned.length(); ++i) {
        char c = cleaned[i];
        if (c >= 'A' && c <= 'Z') {
            if (i > 0 && cleaned[i - 1] != '_') {
                result += '_';
            }
            result += (char)(c + ('a' - 'A'));
        } else {
            result += c;
        }
    }
    return result;
}

bool resolveItem(const std::string& rawItemStr, ResolvedItem& outItem) {
    std::string itemStr = toLower(rawItemStr);
    if (itemStr.rfind("minecraft:", 0) == 0) {
        itemStr = itemStr.substr(10);
    }

    bool isNumeric = !itemStr.empty();
    for (size_t i = 0; i < itemStr.length(); ++i) {
        if (itemStr[i] < '0' || itemStr[i] > '9') {
            isNumeric = false;
            break;
        }
    }

    if (isNumeric) {
        int id = std::atoi(itemStr.c_str());
        if (id >= 0 && id < 256 && Tile::tiles[id] != NULL) {
            outItem.id = id;
            return true;
        } else if (id >= 0 && id < Item::MAX_ITEMS && Item::items[id] != NULL) {
            outItem.id = id;
            return true;
        }
        return false;
    }

    std::map<std::string, int> nameToId;

    for (int i = 0; i < 256; ++i) {
        if (Tile::tiles[i] != NULL) {
            std::string descId = Tile::tiles[i]->getDescriptionId();
            std::string keySnake = cleanDescriptionId(descId);
            std::string keyRaw = getRawDescId(descId);
            if (!keySnake.empty()) nameToId[keySnake] = i;
            if (!keyRaw.empty()) nameToId[keyRaw] = i;
        }
    }

    for (int i = 256; i < Item::MAX_ITEMS; ++i) {
        if (Item::items[i] != NULL) {
            std::string descId = Item::items[i]->getDescriptionId();
            std::string keySnake = cleanDescriptionId(descId);
            std::string keyRaw = getRawDescId(descId);
            if (!keySnake.empty()) nameToId[keySnake] = i;
            if (!keyRaw.empty()) nameToId[keyRaw] = i;
        }
    }

    // Common Java Edition aliases mapping
    if (Tile::rock) nameToId["stone"] = Tile::rock->id;
    if (Tile::rock) nameToId["cobblestone"] = Tile::rock->id;
    if (Tile::dirt) nameToId["dirt"] = Tile::dirt->id;
    if (Tile::grass) nameToId["grass"] = Tile::grass->id;
    if (Tile::grass) nameToId["grass_block"] = Tile::grass->id;
    if (Tile::wood) nameToId["oak_planks"] = Tile::wood->id;
    if (Tile::wood) nameToId["planks"] = Tile::wood->id;
    if (Tile::sand) nameToId["sand"] = Tile::sand->id;
    if (Tile::gravel) nameToId["gravel"] = Tile::gravel->id;
    if (Tile::treeTrunk) nameToId["oak_log"] = Tile::treeTrunk->id;
    if (Tile::treeTrunk) nameToId["log"] = Tile::treeTrunk->id;
    if (Tile::glass) nameToId["glass"] = Tile::glass->id;
    if (Tile::cloth) nameToId["wool"] = Tile::cloth->id;
    if (Tile::cloth) nameToId["white_wool"] = Tile::cloth->id;
    if (Tile::goldBlock) nameToId["gold_block"] = Tile::goldBlock->id;
    if (Tile::ironBlock) nameToId["iron_block"] = Tile::ironBlock->id;
    if (Tile::emeraldBlock) nameToId["diamond_block"] = Tile::emeraldBlock->id;
    if (Tile::emeraldBlock) nameToId["emerald_block"] = Tile::emeraldBlock->id;
    if (Tile::lapisBlock) nameToId["lapis_block"] = Tile::lapisBlock->id;
    if (Tile::sandStone) nameToId["sandstone"] = Tile::sandStone->id;
    if (Tile::bookshelf) nameToId["bookshelf"] = Tile::bookshelf->id;
    if (Tile::mossStone) nameToId["mossy_cobblestone"] = Tile::mossStone->id;
    if (Tile::obsidian) nameToId["obsidian"] = Tile::obsidian->id;
    if (Tile::torch) nameToId["torch"] = Tile::torch->id;
    if (Tile::chest) nameToId["chest"] = Tile::chest->id;
    if (Tile::workBench) nameToId["crafting_table"] = Tile::workBench->id;
    if (Tile::furnace) nameToId["furnace"] = Tile::furnace->id;
    if (Tile::tnt) nameToId["tnt"] = Tile::tnt->id;
    if (Tile::cactus) nameToId["cactus"] = Tile::cactus->id;
    if (Tile::clay) nameToId["clay_block"] = Tile::clay->id;
    if (Tile::ladder) nameToId["ladder"] = Tile::ladder->id;
    if (Tile::snow) nameToId["snow_block"] = Tile::snow->id;
    if (Tile::ice) nameToId["ice"] = Tile::ice->id;

    if (Tile::netherReactor) {
        nameToId["nether_reactor_core"] = Tile::netherReactor->id;
        nameToId["nether_reactor"] = Tile::netherReactor->id;
        nameToId["netherreactor"] = Tile::netherReactor->id;
    }

    if (Item::apple) nameToId["apple"] = Item::apple->id;
    if (Item::stick) nameToId["stick"] = Item::stick->id;
    if (Item::coal) nameToId["coal"] = Item::coal->id;
    if (Item::ironIngot) nameToId["iron_ingot"] = Item::ironIngot->id;
    if (Item::goldIngot) nameToId["gold_ingot"] = Item::goldIngot->id;
    if (Item::emerald) nameToId["diamond"] = Item::emerald->id;
    if (Item::emerald) nameToId["emerald"] = Item::emerald->id;
    if (Item::sword_wood) nameToId["wooden_sword"] = Item::sword_wood->id;
    if (Item::shovel_wood) nameToId["wooden_shovel"] = Item::shovel_wood->id;
    if (Item::pickAxe_wood) nameToId["wooden_pickaxe"] = Item::pickAxe_wood->id;
    if (Item::hatchet_wood) nameToId["wooden_axe"] = Item::hatchet_wood->id;
    if (Item::sword_stone) nameToId["stone_sword"] = Item::sword_stone->id;
    if (Item::shovel_stone) nameToId["stone_shovel"] = Item::shovel_stone->id;
    if (Item::pickAxe_stone) nameToId["stone_pickaxe"] = Item::pickAxe_stone->id;
    if (Item::hatchet_stone) nameToId["stone_axe"] = Item::hatchet_stone->id;
    if (Item::sword_iron) nameToId["iron_sword"] = Item::sword_iron->id;
    if (Item::shovel_iron) nameToId["iron_shovel"] = Item::shovel_iron->id;
    if (Item::pickAxe_iron) nameToId["iron_pickaxe"] = Item::pickAxe_iron->id;
    if (Item::hatchet_iron) nameToId["iron_axe"] = Item::hatchet_iron->id;
    if (Item::sword_emerald) nameToId["diamond_sword"] = Item::sword_emerald->id;
    if (Item::shovel_emerald) nameToId["diamond_shovel"] = Item::shovel_emerald->id;
    if (Item::pickAxe_emerald) nameToId["diamond_pickaxe"] = Item::pickAxe_emerald->id;
    if (Item::hatchet_emerald) nameToId["diamond_axe"] = Item::hatchet_emerald->id;
    if (Item::sword_gold) nameToId["golden_sword"] = Item::sword_gold->id;
    if (Item::shovel_gold) nameToId["golden_shovel"] = Item::shovel_gold->id;
    if (Item::pickAxe_gold) nameToId["golden_pickaxe"] = Item::pickAxe_gold->id;
    if (Item::hatchet_gold) nameToId["golden_axe"] = Item::hatchet_gold->id;
    if (Item::bow) nameToId["bow"] = Item::bow->id;
    if (Item::arrow) nameToId["arrow"] = Item::arrow->id;
    if (Item::bread) nameToId["bread"] = Item::bread->id;
    if (Item::wheat) nameToId["wheat"] = Item::wheat->id;
    if (Item::seeds_wheat) nameToId["wheat_seeds"] = Item::seeds_wheat->id;
    if (Item::seeds_wheat) nameToId["seeds"] = Item::seeds_wheat->id;
    if (Item::flintAndSteel) nameToId["flint_and_steel"] = Item::flintAndSteel->id;
    if (Item::string) nameToId["string"] = Item::string->id;
    if (Item::feather) nameToId["feather"] = Item::feather->id;
    if (Item::sulphur) nameToId["gunpowder"] = Item::sulphur->id;
    if (Item::sulphur) nameToId["sulphur"] = Item::sulphur->id;
    if (Item::flint) nameToId["flint"] = Item::flint->id;
    if (Item::bed) nameToId["bed"] = Item::bed->id;
    if (Item::dye_powder) nameToId["dye"] = Item::dye_powder->id;
    if (Item::bone) nameToId["bone"] = Item::bone->id;
    if (Item::sugar) nameToId["sugar"] = Item::sugar->id;
    if (Item::melon) nameToId["melon_slice"] = Item::melon->id;
    if (Item::melon) nameToId["melon"] = Item::melon->id;

    auto it = nameToId.find(itemStr);
    if (it != nameToId.end()) {
        outItem.id = it->second;
        return true;
    }

    return false;
}

} // namespace

CommandClear::CommandClear() : Command("clear") {}

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
