# Mobs & World Systems Guide

This guide explains how monsters, animals, world generation, biomes, weather, day/night cycles, and sky rendering work in the game engine.

---

## 1. How Mobs Work in the Engine

Every mob in the game (such as Zombies, Skeletons, Pigs, or Cows) is a 3D `Entity` that combines three main components:

```
+-------------------------------------------------------------------------+
|                           MOB ARCHITECTURE                              |
|                                                                         |
|  1. LOGIC & AI CLASS            2. 3D MODEL              3. RENDERER    |
|  (e.g., RubyGolem.cpp)        (RubyGolemModel.cpp)   (RubyGolemRenderer)|
|  +----------------------+    +--------------------+  +------------------+
|  | Health & Attack      |    | 3D Box Parts       |  | Binds PNG Texture|
|  | Pathfinding AI       | -> | Rotates Arms/Legs  |->| Renders Model in |
|  | Item Drops & Loot    |    | Defines UV Mapping |  | 3D World Scene   |
|  +----------------------+    +--------------------+  +------------------+
+-------------------------------------------------------------------------+
```

---

## 2. Step-by-Step Guide: Adding a Custom Mob ("Ruby Golem")

### Step 1: Create Mob Entity Class (`src/world/entity/monster/RubyGolem.h` & `.cpp`)
Create a custom entity class inheriting from `Monster` (hostile) or `Mob` (passive):

**Header (`src/world/entity/monster/RubyGolem.h`):**
```cpp
#pragma once
#include "Monster.h"

class RubyGolem : public Monster {
public:
    RubyGolem(Level* level);

    virtual void aiStep() override;
    virtual int getMaxHealth() override { return 100; }
    virtual int getBaseAttackDamage() { return 8; }
    virtual void dropDeathLoot() override;
};
```

**Source (`src/world/entity/monster/RubyGolem.cpp`):**
```cpp
#include "RubyGolem.h"
#include "world/item/Item.h"

RubyGolem::RubyGolem(Level* level) : Monster(level) {
    this->textureName = "mob/ruby_golem.png";
    this->setSize(1.4f, 2.9f); // Width, Height in blocks
    this->health = getMaxHealth();
}

void RubyGolem::aiStep() {
    Monster::aiStep(); // Handles pathfinding and target searching
}

void RubyGolem::dropDeathLoot() {
    // Drop 1 to 3 Rubies upon death
    int count = random.nextInt(3) + 1;
    for (int i = 0; i < count; i++) {
        spawnAtLocation(Item::ruby->id, 1);
    }
}
```

### Step 2: Create 3D Box Model (`src/client/model/RubyGolemModel.h`)
Models are built using textured 3D boxes called `ModelPart` (or `Cube`):

```cpp
#pragma once
#include "Model.h"
#include "ModelPart.h"

class RubyGolemModel : public Model {
public:
    ModelPart head;
    ModelPart body;
    ModelPart rightArm;
    ModelPart leftArm;

    RubyGolemModel()
        : head(0, 0), body(0, 40), rightArm(60, 0), leftArm(60, 0) {
        // ModelPart(textureOffsetX, textureOffsetY)
        head.addBox(-4.0f, -12.0f, -5.0f, 8, 10, 8); // x, y, z, width, height, depth
        body.addBox(-9.0f, -2.0f, -6.0f, 18, 12, 11);
    }

    virtual void render(Entity* entity, float time, float r, float bob, float yRot, float xRot, float scale) override {
        head.render(scale);
        body.render(scale);
        rightArm.render(scale);
        leftArm.render(scale);
    }
};
```

### Step 3: Create Mob Renderer (`src/client/renderer/entity/RubyGolemRenderer.h`)
Renderers connect the mob's C++ logic to its 3D model and PNG texture:

```cpp
#pragma once
#include "MobRenderer.h"
#include "client/model/RubyGolemModel.h"

class RubyGolemRenderer : public MobRenderer {
public:
    RubyGolemRenderer() : MobRenderer(new RubyGolemModel(), 0.5f) {}
};
```

---

## 3. Biomes & World Generation Pipeline

World generation is split into four distinct phases:

```
+------------------------------------------------------------------------+
|                      WORLD GENERATION PIPELINE                         |
|                                                                        |
|  1. NOISE & TERRAIN   2. BIOME MAPPING    3. SURFACE BLOCKS  4.FEATURES|
|  +-----------------+  +----------------+  +--------------+   +--------+|
|  | PerlinNoise     |->| Temperature    |->| Top: Grass   |-->| Trees  ||
|  | Calculates 3D   |  | & Rainfall     |  | Filler: Dirt |   | Ores   ||
|  | Height Maps     |  | Map            |  | Base: Stone  |   | Caves  ||
|  +-----------------+  +----------------+  +--------------+   +--------+|
+------------------------------------------------------------------------+
```

1. **Terrain Shape (`RandomLevelSource.cpp`):** Uses 3D `PerlinNoise` to generate hills, mountains, and valleys.
2. **Biome Assignment (`BiomeSource.cpp`):** Maps temperature and downfall values across world coordinates to pick biomes.
3. **Surface Blocks (`Biome.cpp`):** Fills top layer with `topBlock` (Grass, Sand) and underlying layer with `fillerBlock` (Dirt, Sandstone).
4. **World Features (`Feature.cpp`):** Places structures like trees (`TreeFeature`), ores (`OreFeature`), and flowers during chunk decoration.

---

## 4. Day/Night Cycle, Weather & Fog

### Time Cycle (`Level::tick()`)
* World time increments by 1 every tick ($20 \text{ ticks/second}$).
* A full Minecraft day lasts 24,000 ticks ($20 \text{ minutes}$).
* `Level::getTimeOfDay(float partialTicks)` returns a value from `0.0f` to `1.0f` representing sun angle and ambient light level.

### Fog & Sky Colors
* **Day Fog:** Bright blue fog blended with biome sky color (`Biome::getSkyColor`).
* **Night Fog:** Dark indigo/black fog.
* **Sunrise/Sunset:** Blends a reddish-gold gradient during twilight transitions.
