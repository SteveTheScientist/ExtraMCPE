# Modding Guide: Adding a New Block

## 1. Quick Checklist
To add a new block (e.g. "Ruby Block"), edit these files in order:
1. `src/world/level/tile/Tile.h` - Declare static `Tile* rubyBlock;` pointer.
2. `src/world/level/tile/Tile.cpp` - Define `rubyBlock = NULL`, instantiate with ID, texture index, material, hardness, sound, and description ID.
3. `src/client/renderer/TileTessellator.cpp` - (Optional) Customize 3D render shape logic if non-standard.
4. `src/world/entity/player/Inventory.cpp` - Add `addItem(new ItemInstance(Tile::rubyBlock));` in `setupDefault()` for Creative inventory.
5. `src/world/item/PickaxeItem.cpp` - Add to pickaxe mining list/min tier if mined by pickaxes.
6. `src/world/item/crafting/Recipes.cpp` - Add crafting recipes.
7. `data/images/terrain.png` - Add the 16x16 tile texture to the terrain atlas.

---

## 2. Worked Example: Ruby Block Step-by-Step

### Declarations in `src/world/level/tile/Tile.h`
```cpp
class Tile {
public:
    static Tile* rubyBlock; // Declare pointer
};
```

### Registration & Properties in `src/world/level/tile/Tile.cpp`
```cpp
// Static definition
Tile* Tile::rubyBlock = NULL;

// In Tile::initTiles():
// ID = 165, Texture Index = 224 (Column 0, Row 14 in 16x16 terrain.png)
rubyBlock = (new MetalTile(165, 224))
    ->init()
    ->setDestroyTime(5.0f)                  // Hardness
    ->setExplodeable(10.0f)                 // Explosion resistance
    ->setSoundType(SOUND_METAL)             // Step & break sound
    ->setCategory(ItemCategory::Decorations)
    ->setDescriptionId("blockRuby");        // Unlocalized description name
```

* **Sound Types:** `SOUND_NORMAL` (stone), `SOUND_WOOD`, `SOUND_GRAVEL`, `SOUND_GRASS`, `SOUND_METAL`, `SOUND_GLASS`.
* **Light Level:** `->setLightEmission(1.0f)` (max 1.0f = 15).
* **Mining Tool & Drops:** For pickaxe blocks, register in `src/world/item/PickaxeItem.cpp`:
```cpp
// In PickaxeItem constructor:
d.push_back(Tile::rubyBlock);
```
Default drops return the tile ID itself (`getResource(int data, Random* rand)` in `Tile.cpp`).

---

## 3. Textures & Atlas Calculation
Block textures are loaded from `data/images/terrain.png`, a $256 \times 256$ pixel atlas grid of $16 \times 16$ tile slots ($16 \times 16 = 256$ total slots).

* **Index Formula:** $\text{texIndex} = (\text{row} \times 16) + \text{col}$.
* **Example:** Column 0, Row 14 $\rightarrow 14 \times 16 + 0 = 224$.

### Multi-Sided Textures
Override `getTexture(int face)` or `getTexture(int face, int data)` in a `Tile` subclass:
```cpp
// face: 0=Bottom, 1=Top, 2=North, 3=South, 4=West, 5=East
int WorkbenchTile::getTexture(int face) {
    if (face == 0) return Tile::wood->getTexture(0); // Bottom
    if (face == 1) return 43;                        // Top
    if (face == 2 || face == 3) return 59;           // Front/Back
    return 60;                                       // Sides
}
```

---

## 4. Creative Inventory, Hotbar & Crafting

### Creative Inventory
Add to `setupDefault()` in `src/world/entity/player/Inventory.cpp`:
```cpp
addItem(new ItemInstance(Tile::rubyBlock));
```

### Hotbar Icon
Blocks use `ItemInstance(Tile*)` and are rendered as miniature 3D isometric cubes automatically by `ItemRenderer::renderGuiItem` in GUI slots and hotbars.

---

## 5. Block Render Shapes
Render shapes dictate the 3D geometry generated in `TileTessellator::tessellateInWorld`:
```cpp
int Tile::getRenderShape() { return SHAPE_BLOCK; }
```
Common shapes defined in `src/world/level/tile/Tile.h`:
* `SHAPE_BLOCK = 0` - Standard 6-sided solid cube.
* `SHAPE_CROSS_TEXTURE = 1` - Intersecting 2D planes (flowers, saplings, tall grass).
* `SHAPE_TORCH = 2` - Torch mesh angled against walls.
* `SHAPE_STAIRS = 10` - Staircase step geometry.
* `SHAPE_FENCE = 11` - Connecting post and rails.

---

## 6. World Storage & ID Limits
* World chunks store block IDs as 8-bit unsigned bytes (`unsigned char` in `LevelChunk.cpp`).
* **ID Limit:** IDs must be $\le 255$. IDs $> 255$ overflow byte storage and break world serialization.
* Metadata/Data values are stored in 4-bit nibbles ($0..15$) in chunk metadata arrays.

---

## 7. Common Pitfalls & Quick Testing
* **Wrong Atlas Index:** Off-by-one or row miscalculations cause blocks to display pink/missing textures or wrong icons.
* **Missing Description ID:** Forgetting `setDescriptionId` causes unlocalized string names in inventories.
* **Build File Inconsistency:** Adding a new `.cpp` subclass for a tile requires adding it to `CMakeLists.txt`, `project/android/jni/Android.mk`, `project/android_java/jni/Android.mk`, and `project/iosproj/minecraftpe.xcodeproj/project.pbxproj`.
* **Quick Testing:** Add `Tile::rubyBlock` to hotbar slot 0 in `Inventory::setupDefault()`:
```cpp
Sel[0] = addItem(new ItemInstance(Tile::rubyBlock));
```
Then launch `MinecraftPE-server` or `MinecraftPE` to verify instantly on spawn.
