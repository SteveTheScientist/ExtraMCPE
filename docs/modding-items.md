# Modding Guide: Adding Items & Gameplay Features

## 1. Quick Checklist
To add a new item (e.g. "Ruby"), edit these files in order:
1. `src/world/item/Item.h` - Declare static pointer `static Item* ruby;`.
2. `src/world/item/Item.cpp` - Define `ruby = NULL`, instantiate with ID ($> 256$), set icon (`setIcon(x, y)`), set category, and set description ID.
3. `src/world/entity/player/Inventory.cpp` - Add `addItem(new ItemInstance(Item::ruby));` in `setupDefault()`.
4. `data/images/items.png` - Add the $16 \times 16$ item texture to the item atlas grid.
5. `src/world/item/crafting/Recipes.cpp` / `FurnaceRecipes.cpp` - Add crafting or smelting recipes.

---

## 2. Adding a Simple Item ("Ruby")

### Declarations in `src/world/item/Item.h`
```cpp
class Item {
public:
    static Item* ruby; // Declare item pointer
};
```

### Registration in `src/world/item/Item.cpp`
Item IDs start after block IDs ($> 256$); the constructor takes an offset ID $N$, resulting in actual item ID $256 + N$.
```cpp
Item* Item::ruby = NULL; // Definition

// In Item::initItems():
ruby = (new Item(100))                     // Actual ID = 256 + 100 = 356
    ->setIcon(0, 4)                        // Column 0, Row 4 in items.png
    ->setCategory(ItemCategory::Items)     // Inventory tab category
    ->setDescriptionId("ruby")             // Unlocalized name
    ->setMaxStackSize(64);                 // Stack limit
```

### Texture Calculation
Item textures are loaded from `data/images/items.png` ($256 \times 256$ grid of $16 \times 16$ icon slots).
* `setIcon(col, row)`: `setIcon(0, 4)` targets Column 0, Row 4 ($x=0, y=64$ px).

---

## 3. Food Items ("Ruby Apple")
Food items inherit from `FoodItem` (`src/world/item/FoodItem.h`):
```cpp
// Constructor: FoodItem(idOffset, healAmount, isMeat)
rubyApple = (new FoodItem(101, 8, false))
    ->setIcon(10, 0)
    ->setCategory(ItemCategory::FoodArmor)
    ->setDescriptionId("rubyApple");
```
* **Duration & Animation:** `FoodItem` overrides `getUseAnimation()` to return `UseAnim::eat` or `drink` and uses `getUseDuration()` (default 32 ticks). Eating particle effects and sounds (`random.eat`) are automatically handled in `Player::spawnEatParticles` and `LocalPlayer::tick()`.

---

## 4. Tools, Weapons & Armor

### Material Tiers (`Item::Tier`)
Tools use `Item::Tier` (`src/world/item/Item.h`):
* `EMERALD`: Level 3, Uses 1561, Speed 8.0f, Damage 3
* `IRON`: Level 2, Uses 250, Speed 6.0f, Damage 2
* `STONE`: Level 1, Uses 131, Speed 4.0f, Damage 1

### Sword Example ("Ruby Sword")
```cpp
rubySword = (new SwordItem(102, Item::Tier::EMERALD))
    ->setIcon(2, 4)
    ->setCategory(ItemCategory::ToolsWeapons)
    ->setDescriptionId("swordRuby");
```
Attack damage is computed via tier damage + weapon base damage in `SwordItem::getAttackDamage`.

---

## 5. Crafting & Smelting Recipes

### Shaped Crafting Recipe (`src/world/item/crafting/Recipes.cpp`)
```cpp
addShapedRecipe(ItemInstance(Item::rubySword, 1),
    "R",
    "R",
    "S",
    {
        Recipe::Type('#', Tile::rock),
        Recipe::Type('R', Item::ruby),
        Recipe::Type('S', Item::stick)
    }
);
```

### Shapeless Crafting Recipe
```cpp
addShapelessRecipe(ItemInstance(Item::rubyApple, 1),
    {
        Recipe::Type(Item::apple),
        Recipe::Type(Item::ruby)
    }
);
```

### Furnace Smelting Recipe (`src/world/item/crafting/FurnaceRecipes.cpp`)
```cpp
// Smelt Ruby Ore tile into Ruby item
addFurnaceRecipe(Tile::rubyOre->id, ItemInstance(Item::ruby, 1));
```

---

## 6. Drops & Loot

### Block Drops (`src/world/level/tile/Tile.cpp`)
Override `getResource` and `getResourceCount` on a `Tile`:
```cpp
int RubyOreTile::getResource(int data, Random* rand) {
    return Item::ruby->id; // Drop ruby item instead of block
}
int RubyOreTile::getResourceCount(Random* rand) {
    return 1 + rand->nextInt(2); // Drop 1-2 rubies
}
```

### Mob Drops (`src/world/entity/monster/Monster.cpp`)
Override `dropDeathLoot` on a mob class:
```cpp
void Zombie::dropDeathLoot() {
    int count = random.nextInt(3);
    for (int i = 0; i < count; i++) {
        spawnAtLocation(Item::ruby->id, 1);
    }
}
```

---

## 7. Placing Items as Blocks & Right-Click Use
* **Tile Items:** To make an item place a block, use `TileItem` (`src/world/item/TileItem.h`):
```cpp
rubyItem = (new TileItem(103, Tile::rubyBlock->id))
    ->setIcon(0, 4)
    ->setDescriptionId("rubyBlock");
```
* **Right-Click Actions:** Override `useOn` or `use` in a custom `Item` subclass:
```cpp
bool Item::useOn(ItemInstance* instance, Player* player, Level* level, int x, int y, int z, int side, float ex, float ey, float ez) {
    // Return true if action succeeded (e.g., placing, lighting fire)
    return false;
}
```

---

## 8. Sounds & Translations
* **Description ID:** Set via `setDescriptionId("ruby")` -> mapped internally to `item.ruby.name`.
* **Sound Triggers:** Sounds are triggered via `level->playSound(player, "random.pop", 0.2f, 1.0f)`. Common sound keys: `"random.pop"`, `"random.drink"`, `"random.eat"`, `"random.break"`.

---

## 9. Common Mistakes & Quick Testing
* **Item ID Collision:** Using IDs $\le 256$ overwrites block IDs. Always use $N$ so actual ID is $256 + N$.
* **Missing Icon Index:** Invalid `setIcon(x, y)` grid coordinates display broken textures.
* **Missing Build Entries:** If you create a new `.cpp` file for a custom item subclass, add it to `CMakeLists.txt`, `project/android/jni/Android.mk`, `project/android_java/jni/Android.mk`, and `project/iosproj/minecraftpe.xcodeproj/project.pbxproj`.
* **Quick Testing:** Add item to hotbar slot 0 in `src/world/entity/player/Inventory.cpp`:
```cpp
Sel[0] = addItem(new ItemInstance(Item::ruby));
```
