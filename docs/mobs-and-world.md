# Mobs & World Systems Guide

## 1. Adding a New Custom Mob (e.g. "Ruby Golem")

### Checklist & Step-by-Step Instructions
1. **Entity Class (`src/entity/RubyGolem.h`, `src/entity/RubyGolem.cpp`):**
   * Inherit from `Mob` (`src/entity/Mob.h`) or `Monster` (`src/entity/Monster.h`).
   * Implement `aiStep()`, `getMobType()`, `getMaxHealth()`, and `getDropItem()`.
2. **Model Class (`src/client/model/RubyGolemModel.h`):**
   * Inherit from `Model` (`src/client/model/Model.h`).
   * Define body parts using `Cube` (`src/client/model/Cube.h`) and set UV offsets in `render(Entity*, float, float, float, float, float, float)`.
3. **Renderer Class (`src/client/renderer/entity/RubyGolemRenderer.h`):**
   * Inherit from `MobRenderer` (`src/client/renderer/entity/MobRenderer.h`).
   * Pass `RubyGolemModel` instance and texture path string (e.g. `"mob/ruby_golem.png"`) to constructor.
4. **Register Mob ID (`src/entity/EntityIds.h` / `src/entity/Entity.h`):**
   * Add `RubyGolem = <unique_id>` to entity ID enum.
5. **Register Model & Renderer (`src/client/renderer/entity/EntityRenderDispatcher.cpp`):**
   * Register renderer instance in `EntityRenderDispatcher` map.
6. **Register Factory Spawner (`src/entity/EntityFactory.cpp`):**
   * Map ID to constructor in `EntityFactory::CreateEntity`.
7. **Add Biome Spawn Rules (`src/level/biome/Biome.h`):**
   * Add `SpawnListEntry(RubyGolem::class, weight, min, max)` to `biome->getMobs(MobCategory::monster)`.

### Code Example: `RubyGolem` Entity Class
```cpp
#include "entity/Monster.h"

class RubyGolem : public Monster {
public:
    RubyGolem(Level* level) : Monster(level) {
        this->texture = "mob/ruby_golem.png";
        this->setSize(1.4f, 2.9f);
    }
    virtual int getMaxHealth() override { return 100; }
    virtual int getDropItem() override { return 500; } // Ruby Item ID
};
```

## 2. Biomes & World Generation Pipeline

### World Generation Architecture
1. **`Level` (`src/level/Level.h`):** Main container managing chunk storage, entity updates, and tick loops.
2. **`ChunkSource` / `RandomLevelSource` (`src/level/chunk/RandomLevelSource.h`):** Generates terrain depth, surface blocks, and noise maps using `PerlinNoise`.
3. **`Biome` (`src/level/biome/Biome.h`):** Controls surface blocks (`topBlock`, `fillerBlock`), foliage colors, temperature, downfall, and mob spawn lists.
4. **`BiomeSource` (`src/level/biome/BiomeSource.h`):** Provides biome maps across block coordinates using noise layers.
5. **`Feature` (`src/level/feature/Feature.h`):** Generates world structures into chunks during decoration phase (`TreeFeature`, `OreFeature`, `FlowerFeature`).

### Adding a Custom Biome
1. Define new instance in `Biome.cpp` (e.g. `Biome* Biome::rubyPlains = (new Biome(15))->setTemperatureAndDownfall(0.8f, 0.4f);`).
2. Override `topBlock` (`Tile::rubyBlock->id`) and `fillerBlock` (`Tile::dirt->id`).
3. Add custom features in `Biome::decorate(Level* level, Random* rand, int x, int z)`.

## 3. Day/Night Cycle, Weather, Sky, Fog & Clouds

### Day/Night Time Loop
* **`Level::tick()` (`src/level/Level.cpp`):** Increments `time` counter each tick (20 ticks/sec, full cycle = 24000 ticks).
* **`Level::getTimeOfDay(float partialTicks)`:** Returns scalar `0.0f` to `1.0f` used for sky rotation angle and ambient light level calculation.

### Weather System (Rain / Snow)
* **`LevelData` (`src/level/LevelData.h`):** Stores `rainTime`, `isRaining`, `thunderTime`, and `isThundering`.
* **`LevelRenderer::renderSnowAndRain(float partialTicks)` (`src/client/renderer/LevelRenderer.cpp`):** Iterates around player camera, checks `Biome::getDownfall()` and temperature to render rain lines or animated snow particles.

### Sky, Fog & Cloud Rendering
* **Sky Color:** `LevelRenderer::renderSky` computes celestial angle and blends biome sky color (`Biome::getSkyColorByTemp`) with sunrise/sunset gradients.
* **Fog System:** `Dimension::getFogColor(float time, float partialTicks)` (`src/level/dimension/Dimension.cpp`) calculates RGB fog values. `LevelRenderer` applies GL fog equations (`glFogfv(GL_FOG_COLOR, ...)`).
* **Clouds:** `LevelRenderer::renderClouds` renders 2D/3D volumetric cloud plane moving along X/Z axes based on total game ticks.
