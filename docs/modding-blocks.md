# Beginner Guide to Modding & Adding Blocks

This guide explains how blocks work in the game engine and provides step-by-step instructions and complete code examples for adding custom blocks.

---

## 1. How Blocks Work in the Engine

In Minecraft, every block type in the world is managed by a `Tile` object.
* **Block ID (0 to 255):** Every block type in the game receives a unique number ID between `0` and `255`.
* **Block Array (`Tile::tiles[256]`):** A global array where index `i` points to the `Tile` object for block ID `i`. Block ID `0` is reserved for Air (`Tile::tiles[0] == NULL`).
* **Block Metadata (4-bit Data Value, 0 to 15):** A single block ID can store extra state information (such as orientation, wool color, or crop growth stage) using 4 bits of metadata (`0` to `15`).

```
+----------------------------------------------------------------------+
|                         TILE / BLOCK ARCHITECTURE                    |
|                                                                      |
|  Tile::tiles[256] Array                                              |
|  +----+------------------+---------------------+-------------------+ |
|  | ID | Block Pointer    | Properties          | Texture Offset    | |
|  +----+------------------+---------------------+-------------------+ |
|  | 0  | NULL (Air)       | Non-solid, Light=0  | None              | |
|  | 1  | Tile::rock       | Solid, Hardness=1.5 | Index 1 (Stone)   | |
|  | 2  | Tile::grass      | Solid, Hardness=0.6 | Top=0, Side=3...  | |
|  | ...| ...              | ...                 | ...               | |
|  | 200| Tile::rubyBlock  | Solid, Hardness=5.0 | Index 180         | |
|  +----+------------------+---------------------+-------------------+ |
+----------------------------------------------------------------------+
```

---

## 2. Step-by-Step Guide: Adding a Simple Solid Block ("Ruby Block")

Here is how to create a new solid block called **Ruby Block**:

### Step 1: Declare Block Pointer (`src/world/level/tile/Tile.h`)
Open `src/world/level/tile/Tile.h` and declare the static pointer inside the `Tile` class:
```cpp
class Tile {
public:
    static Tile* rubyBlock; // Declare pointer for Ruby Block
};
```

### Step 2: Instantiate Block (`src/world/level/tile/Tile.cpp`)
Open `src/world/level/tile/Tile.cpp`:
1. Define the static variable near the top:
```cpp
Tile* Tile::rubyBlock = NULL;
```
2. Allocate and register the block inside `Tile::initTiles()`:
```cpp
// Constructor parameters: Tile(BlockID, Material)
// Block ID 200, uses metal sound, 5.0f mining hardness, 10.0f blast resistance
rubyBlock = (new Tile(200, Material::metal))
    ->setDestroyTime(5.0f)
    ->setExplodeable(10.0f)
    ->setSoundType(Tile::SOUND_METAL)
    ->setDescriptionId("rubyBlock");

rubyBlock->tex = 180; // Texture grid index in terrain.png
```

### Step 3: Register Block Display Name (`data/lang/en_US.lang`)
Add the localized name to `data/lang/en_US.lang`:
```ini
tile.rubyBlock.name=Ruby Block
```

### Step 4: Register Block as Inventory Item (`src/world/item/Item.cpp`)
In `Item::initItems()`, register a `TileItem` so the block can be held in player inventory:
```cpp
// TileItem wraps Block ID (200 - 256 = -56) as an item
new TileItem(Tile::rubyBlock->id - 256);
```

### Step 5: Add Block to Creative Inventory (`src/world/entity/player/Inventory.cpp`)
In `Inventory::setupDefault()` inside `src/world/entity/player/Inventory.cpp`:
```cpp
addItem(new ItemInstance(Tile::rubyBlock));
```

---

## 3. Custom Block Properties & Behaviors

You can configure block properties using builder methods:

| Method | Description | Example |
| :--- | :--- | :--- |
| `setDestroyTime(float hardness)` | Mining time in seconds | `->setDestroyTime(1.5f)` (Stone) |
| `setExplodeable(float resistance)` | Resistance to TNT explosions | `->setExplodeable(10.0f)` |
| `setSoundType(SoundType sound)` | Step & break sounds (`SOUND_STONE`, `SOUND_WOOD`, `SOUND_METAL`, `SOUND_GRASS`) | `->setSoundType(Tile::SOUND_WOOD)` |
| `setLightEmission(float level)` | Makes block glow in dark (`0.0f` to `1.0f`) | `->setLightEmission(1.0f)` (Glows like Torch) |
| `setLightBlock(int level)` | Light blocked when passing through (`0` to `255`) | `->setLightBlock(0)` (Transparent) |

---

## 4. Custom Block Classes (e.g. Ores that Drop Items)

If you want a block with custom drops (like **Ruby Ore** dropping a **Ruby** item instead of itself), create a custom class inheriting from `Tile`.

### Header File (`src/world/level/tile/RubyOreTile.h`)
```cpp
#pragma once
#include "Tile.h"

class RubyOreTile : public Tile {
public:
    RubyOreTile(int id, const Material* material) : Tile(id, material) {}

    // Return custom item ID dropped when mined
    virtual int getResource(int data, Random* random) override;

    // Return quantity of items dropped
    virtual int getResourceCount(Random* random) override;
};
```

### Source File (`src/world/level/tile/RubyOreTile.cpp`)
```cpp
#include "RubyOreTile.h"
#include "world/item/Item.h"

int RubyOreTile::getResource(int data, Random* random) {
    return Item::ruby->id; // Drop Ruby item instead of block
}

int RubyOreTile::getResourceCount(Random* random) {
    return 1 + random->nextInt(2); // Drop 1 to 2 Rubies
}
```

---

## 5. Texture Atlas Grid System (`terrain.png`)

Textures for blocks are packed in a $16 \times 16$ grid inside `data/images/terrain.png`.

```
          COLUMN (0 to 15)
        0   1   2   3 ... 15
      +---+---+---+---+---+---+
    0 |   |   |   |   |   |   |  texIndex = column + (row * 16)
R   1 |   |   |   |   |   |   |
O   2 |   |   |   |   |   |   |  Example: Column 4, Row 11
W   3 |   |   |   |   |   |   |  texIndex = 4 + (11 * 16) = 180
  ... |   |   |   |   |   |   |
   15 |   |   |   |   |   |   |
      +---+---+---+---+---+---+
```

### Multi-Face Textures (e.g., Grass-style block)
To display different textures on Top, Bottom, and Sides, override `getTexture(int face, int data)` in your block class:

```cpp
// face: 0 = Bottom, 1 = Top, 2 = North, 3 = South, 4 = West, 5 = East
int RubyBlockTile::getTexture(int face, int data) override {
    if (face == 1) return 181; // Top texture
    if (face == 0) return 182; // Bottom texture
    return 180;                // Side textures
}
```

---

## 6. Render Shapes & Bounding Boxes

To change the 3D shape of a block, override `getRenderShape()`:

| Shape ID | Description | Examples |
| :--- | :--- | :--- |
| `0` | Standard $1 \times 1 \times 1$ Solid Cube | Stone, Dirt, Ores, Plank |
| `1` | Cross-Mesh Plants | Flowers, Mushrooms, Tall Grass |
| `2` | Torch / Wall-mounted mesh | Torch |
| `4` | Liquid animation mesh | Water, Lava |
| `10` | Stairs mesh | Stone Stairs, Wood Stairs |
| `11` | Fence mesh | Wooden Fence |
| `13` | Cactus mesh | Cactus |
| `14` | Bed mesh | Bed |

### Non-Solid / Transparent Blocks (e.g., Glass)
For glass-like blocks, override transparency methods:
```cpp
bool CustomGlassTile::isSolidRender() override { return false; }
bool CustomGlassTile::isCubeShaped() override { return false; }
```
