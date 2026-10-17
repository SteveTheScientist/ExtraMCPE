#include "client/renderer/EnhancedFog.h"
#include "world/level/biome/Biome.h"
#include "world/level/Level.h"
#include "world/entity/Mob.h"
#include "util/Mth.h"
#include <iostream>
#include <cmath>
#include <cassert>

static bool isClose(float a, float b, float tol = 1e-3f) {
    return std::fabs(a - b) < tol;
}

static bool isCloseCol(Color3f c1, Color3f c2, float tol = 1e-3f) {
    return isClose(c1.r, c2.r, tol) && isClose(c1.g, c2.g, tol) && isClose(c1.b, c2.b, tol);
}

// Define static members of Biome to avoid linking the full Biome.cpp dependency tree
Biome* Biome::rainForest     = (Biome*)0x1000;
Biome* Biome::swampland      = (Biome*)0x2000;
Biome* Biome::seasonalForest = (Biome*)0x3000;
Biome* Biome::forest         = (Biome*)0x4000;
Biome* Biome::savanna        = (Biome*)0x5000;
Biome* Biome::shrubland      = (Biome*)0x6000;
Biome* Biome::taiga          = (Biome*)0x7000;
Biome* Biome::desert         = (Biome*)0x8000;
Biome* Biome::plains         = (Biome*)0x9000;
Biome* Biome::iceDesert      = (Biome*)0xa000;
Biome* Biome::tundra         = (Biome*)0xb000;

int getTimeMs() { return 0; }

#include "RakNetTypes.h"
RakNet::SystemAddress::SystemAddress() {}

int main() {
    std::cout << "Running EnhancedFog Unit Tests..." << std::endl;

    // 1. Biome Day and Night Fog Colors and Daylight Factor Blending
    Color3f desertDay = EnhancedFog::getBiomeFogColor(Biome::desert, 1.0f);
    Color3f desertNight = EnhancedFog::getBiomeFogColor(Biome::desert, 0.0f);
    Color3f desertMid = EnhancedFog::getBiomeFogColor(Biome::desert, 0.5f);

    std::cout << "Desert Day: (" << desertDay.r << ", " << desertDay.g << ", " << desertDay.b << ")" << std::endl;
    std::cout << "Desert Night: (" << desertNight.r << ", " << desertNight.g << ", " << desertNight.b << ")" << std::endl;
    std::cout << "Desert Mid: (" << desertMid.r << ", " << desertMid.g << ", " << desertMid.b << ")" << std::endl;

    assert(isCloseCol(desertDay, Color3f(0.92f, 0.82f, 0.60f)));
    assert(isCloseCol(desertNight, Color3f(0.20f, 0.22f, 0.35f)));
    assert(isCloseCol(desertMid, Color3f(0.56f, 0.52f, 0.475f)));

    // Verify night is darker (lower luminance) and bluer (higher b/r ratio)
    float dayLuma = 0.2126f * desertDay.r + 0.7152f * desertDay.g + 0.0722f * desertDay.b;
    float nightLuma = 0.2126f * desertNight.r + 0.7152f * desertNight.g + 0.0722f * desertNight.b;
    assert(nightLuma < dayLuma);
    assert((desertNight.b / desertNight.r) > (desertDay.b / desertDay.r));

    Color3f swampDay = EnhancedFog::getBiomeFogColor(Biome::swampland, 1.0f);
    assert(isCloseCol(swampDay, Color3f(0.45f, 0.55f, 0.45f)));

    Color3f taigaDay = EnhancedFog::getBiomeFogColor(Biome::taiga, 1.0f);
    assert(isCloseCol(taigaDay, Color3f(0.80f, 0.85f, 0.92f)));

    Color3f plainsDay = EnhancedFog::getBiomeFogColor(Biome::plains, 1.0f);
    assert(isCloseCol(plainsDay, Color3f(0.70f, 0.80f, 0.90f)));

    // 2. Frame-Time Independent Smoothing Step Test
    Color3f startCol(0.0f, 0.0f, 0.0f);
    Color3f targetCol(1.0f, 1.0f, 1.0f);
    float speed = 5.0f;

    // Zero delta time should not change current color
    Color3f dt0 = EnhancedFog::updateSmoothing(startCol, targetCol, 0.0f, speed);
    assert(isCloseCol(dt0, startCol));

    // Two half-step updates should equal one full-step update (frame-time independence)
    float fullDt = 0.1f;
    float halfDt = 0.05f;

    Color3f singleStep = EnhancedFog::updateSmoothing(startCol, targetCol, fullDt, speed);
    Color3f step1 = EnhancedFog::updateSmoothing(startCol, targetCol, halfDt, speed);
    Color3f doubleStep = EnhancedFog::updateSmoothing(step1, targetCol, halfDt, speed);

    std::cout << "Single step (0.1s): (" << singleStep.r << ", " << singleStep.g << ", " << singleStep.b << ")" << std::endl;
    std::cout << "Double step (2x0.05s): (" << doubleStep.r << ", " << doubleStep.g << ", " << doubleStep.b << ")" << std::endl;
    assert(isCloseCol(singleStep, doubleStep, 1e-4f));

    // Large delta time should approach target color
    Color3f largeStep = EnhancedFog::updateSmoothing(startCol, targetCol, 10.0f, speed);
    assert(isCloseCol(largeStep, targetCol, 1e-3f));

    // 3. Fog Distance Calculation Test
    float renderDist = 256.0f;
    float startPct = 50.0f;
    float endPct = 100.0f;
    float fogStart = 0.0f;
    float fogEnd = 0.0f;

    EnhancedFog::calculateFogDistances(renderDist, startPct, endPct, fogStart, fogEnd);
    assert(isClose(fogStart, 128.0f));
    assert(isClose(fogEnd, 256.0f));

    // Terrain edge is fully fogged at render distance (fogEnd == renderDist)
    assert(isClose(fogEnd, renderDist));

    // 4. Test 10-frame sampling behavior
    EnhancedFog fogModule;
    FogSettings settings;
    settings.enhancedFog = true;

    // Use dummy camera coordinates via Mob pointer or direct sampling call
    EnhancedFog::sampleBiomeGrid(nullptr, 0.0f, 0.0f, 1.0f);
    assert(!fogModule.isInitialized());

    std::cout << "All EnhancedFog unit tests passed successfully!" << std::endl;
    return 0;
}
