# Beginner Guide to Modding & Adding Items

This guide explains how items, tools, weapons, food, crafting recipes, and translations work in the game engine.

---

## 1. How Items Work in the Engine

In Minecraft, every item in the game is managed by an `Item` object registered in the global array `Item::items[512]`.

```
+----------------------------------------------------------------------+
|                         ITEM ARCHITECTURE                            |
|                                                                      |
|  Block IDs (0 to 255)                Item IDs (256 to 511)          |
|  +-----+-------------------+         +-----+-----------------------+ |
|  | ID  | Block             |         | ID  | Item                  | |
|  +-----+-------------------+         +-----+-----------------------+ |
|  | 1   | Tile::rock        |         | 256 | Item::shovel_iron     | |
|  | 2   | Tile::grass       |         | 257 | Item::pickAxe_iron    | |
|  | ... | ...               |         | ... | ...                   | |
|  | 200 | Tile::rubyBlock   |         | 356 | Item::ruby (ID 256+100)| |
|  +-----+-------------------+         +-----+-----------------------+ |
+----------------------------------------------------------------------+
```

* **ID Offset ($256 + N$):** Standard item constructors take an offset $N$. For example, `new Item(100)` creates an item with ID $256 + 100 = 356$.
* **Stack Size:** Default maximum stack size is 64 items per slot (`maxStackSize = 64`).

---

## 2. Step-by-Step Guide: Adding a Simple Item ("Ruby")

### Step 1: Declare Item Pointer (`src/world/item/Item.h`)
Open `src/world/item/Item.h` and declare the static pointer inside the `Item` class:
```cpp
class Item {
public:
    static Item* ruby; // Declare pointer for Ruby
};
```

### Step 2: Define and Register Item (`src/world/item/Item.cpp`)
Open `src/world/item/Item.cpp`:
1. Define the static pointer near top of file:
```cpp
Item* Item::ruby = NULL;
```
2. Allocate and register item inside `Item::initItems()`:
```cpp
// Offset 100 -> Actual ID = 256 + 100 = 356
ruby = (new Item(100))
    ->setIcon(0, 4)                       // Column 0, Row 4 in items.png
    ->setCategory(ItemCategory::Items)    // Creative inventory tab
    ->setDescriptionId("ruby")            // Translation ID key
    ->setMaxStackSize(64);                // Stack size limit
```

### Step 3: Add Translation Name (`data/lang/en_US.lang`)
Add display name to `data/lang/en_US.lang`:
```ini
item.ruby.name=Ruby
```

### Step 4: Add to Inventory (`src/world/entity/player/Inventory.cpp`)
Add item to Creative inventory inside `Inventory::setupDefault()`:
```cpp
addItem(new ItemInstance(Item::ruby));
```

---

## 3. Creating Food Items ("Ruby Apple")

Food items inherit from `FoodItem` (`src/world/item/FoodItem.h`), which automatically handles eating animations, eating sounds (`random.eat`), and health restoration.

```cpp
// Constructor: FoodItem(idOffset, healAmount, isMeat)
rubyApple = (new FoodItem(101, 8, false))
    ->setIcon(10, 0)
    ->setCategory(ItemCategory::FoodArmor)
    ->setDescriptionId("rubyApple");
```

When consumed, `FoodItem` restores 8 health points ($4$ hearts) and plays eating particle effects.

---

## 4. Creating Tools and Weapons

Tools and weapons use material tier stats defined in `Item::Tier`:

```cpp
// Built-in Tiers:
Item::Tier::WOOD    // Mining Level 0, 59 Uses, Speed 2.0f, Damage Bonus 0
Item::Tier::STONE   // Mining Level 1, 131 Uses, Speed 4.0f, Damage Bonus 1
Item::Tier::IRON    // Mining Level 2, 250 Uses, Speed 6.0f, Damage Bonus 2
Item::Tier::EMERALD // Mining Level 3, 1561 Uses, Speed 8.0f, Damage Bonus 3 (Diamond tier)
Item::Tier::GOLD    // Mining Level 0, 32 Uses, Speed 12.0f, Damage Bonus 0
```

### Adding a Custom Sword ("Ruby Sword")
```cpp
rubySword = (new WeaponItem(102, Item::Tier::EMERALD))
    ->setIcon(2, 4)
    ->setCategory(ItemCategory::Tools)
    ->setDescriptionId("swordRuby");
```

---

## 5. Adding Crafting & Furnace Smelting Recipes

### Shaped Crafting Recipe (`src/world/item/crafting/Recipes.cpp`)
To craft 1 **Ruby Sword** using 2 **Rubies** ("R") and 1 **Stick** ("S"):

```cpp
addShapedRecipe(ItemInstance(Item::rubySword, 1),
    "R",
    "R",
    "S",
    {
        Recipe::Type('R', Item::ruby),
        Recipe::Type('S', Item::stick)
    }
);
```

### Shapeless Crafting Recipe
Ingredients can be placed anywhere in crafting grid:
```cpp
addShapelessRecipe(ItemInstance(Item::rubyApple, 1),
    {
        Recipe::Type(Item::apple),
        Recipe::Type(Item::ruby)
    }
);
```

### Furnace Smelting Recipe (`src/world/item/crafting/FurnaceRecipes.cpp`)
Smelt **Ruby Ore** block into 1 **Ruby** item:
```cpp
addFurnaceRecipe(Tile::rubyOre->id, ItemInstance(Item::ruby, 1));
```

---

## 6. Texture Atlas Grid System (`items.png`)

Item textures are loaded from `data/images/items.png` ($256 \times 256$ grid of $16 \times 16$ pixel icon slots).

```
          COLUMN (0 to 15)
        0   1   2   3 ... 15
      +---+---+---+---+---+---+
    0 |   |   |   |   |   |   |  setIcon(column, row)
R   1 |   |   |   |   |   |   |
O   2 |   |   |   |   |   |   |  Example: Column 0, Row 4
W   3 |   |   |   |   |   |   |  setIcon(0, 4)
    4 | X |   |   |   |   |   |  Targets (x=0px, y=64px)
   15 |   |   |   |   |   |   |
      +---+---+---+---+---+---+
```
