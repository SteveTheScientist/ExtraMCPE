# Modding Blocks Guide

## 1. Adding a New Custom Block (e.g. "Ruby Block")

### Step-by-Step Checklist
1. **Declare Block Instance Pointer (`src/level/tile/Tile.h`):**
   * Add static pointer `static Tile* rubyBlock;` to `Tile` class.
2. **Define ID & Constructor (`src/level/tile/Tile.cpp`):**
   * Allocate new `Tile` instance in `Tile::initTiles()`:
     ```cpp
     rubyBlock = (new Tile(200, Material::metal))->setDestroyTime(5.0f)->setExplodeable(10.0f)->setSoundType(Tile::SOUND_METAL)->setDescriptionId("rubyBlock");
     ```
3. **Assign Texture Index / Atlas Offset:**
   * Set texture index on block using `rubyBlock->tex = 180;` or override `getTexture(int face)`.
4. **Register Translation Key (`assets/lang/en_US.lang`):**
   * Add `tile.rubyBlock.name=Ruby Block` to translation files.
5. **Add ItemTile Entry (`src/item/Item.cpp`):**
   * Initialize `new TileItem(200 - 256)` so block can be held as an item in inventory.
6. **Add Creative Inventory Category (`src/item/Item.cpp`):**
   * Call `Item::addCreativeItem(Tile::rubyBlock->id, 0);`.

## 2. Block Render Shapes & Custom Renderers

### Standard Render Shapes (`Tile::getRenderShape()`)
* `0`: Standard cubic block (1x1x1 solid cube, e.g. Stone, Dirt, Ores).
* `1`: Cross plant mesh (e.g. Flowers, Mushrooms, Tall Grass).
* `2`: Torch / Wall mounted block mesh.
* `3`: Fire particle mesh.
* `4`: Liquid animation mesh (Water, Lava).
* `6`: Wheat / Crops stage mesh.
* `10`: Stairs block mesh.
* `11`: Fence block mesh.
* `13`: Cactus mesh.
* `14`: Bed mesh.
* `16`: Piston / Door mesh.

### Custom Block Renderer (`src/client/renderer/TileTessellator.cpp`)
To create custom geometry (e.g. custom 3D model block):
1. Return custom shape ID in `Tile::getRenderShape()` (e.g. `22`).
2. Add switch case in `TileTessellator::tessellateInWorld(Tile* tile, const TilePos& pos)`:
   ```cpp
   case 22:
       return tessellateRubyBlock(tile, pos);
   ```
3. Use `Tessellator::instance` (`src/client/renderer/Tessellator.h`) to emit vertices, UV coordinates, and face lighting.

## 3. Texture Atlas & UV Coordinates

### Terrain Texture Atlas Architecture (`terrain.png`)
* Textures are packed in a 16x16 grid (`terrain.png` asset file).
* Each tile index `0..255` corresponds to row/column grid offsets:
  * `u0 = (texIndex % 16) / 16.0f`
  * `u1 = u0 + (1.0f / 16.0f)`
  * `v0 = (texIndex / 16) / 16.0f`
  * `v1 = v0 + (1.0f / 16.0f)`

### Custom Multi-Face Textures
Override `Tile::getTexture(int face, int data)`:
```cpp
int RubyBlockTile::getTexture(int face, int data) {
    if (face == 0) return 181; // Bottom face
    if (face == 1) return 182; // Top face
    return 180;                // Side faces
}
```

## 4. World Storage Architecture & Hard ID Limits

### Chunk Storage Format
* Block IDs are stored as `unsigned char` (8-bit integer) in chunk byte arrays (`src/level/chunk/LevelChunk.h`).
* **Hard Block ID Limit:** Exactly `256` total block IDs (`0` to `255`).
  * Block ID `0` is reserved for Air (`Tile::tiles[0] == nullptr`).
  * Native IDs above 255 cannot be stored in standard chunk byte arrays without breaking world save compatibility.

### Metadata / Block Data
* Block metadata stored as 4-bit nibbles (`0..15`) in `chunk->data`.
* Use metadata for block states (rotations, open/closed doors, growth stages) rather than consuming multiple block IDs.
