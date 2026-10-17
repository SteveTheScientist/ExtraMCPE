#include "EnhancedFog.h"
#include "../../world/level/Level.h"
#include "../../world/level/biome/Biome.h"
#include "../../world/entity/Mob.h"
#include "../../util/Mth.h"
#include <cmath>
#include <algorithm>

EnhancedFog::EnhancedFog()
    : m_currentColor(0.8f, 0.85f, 0.9f),
      m_targetColor(0.8f, 0.85f, 0.9f),
      m_cachedDayColor(0.8f, 0.85f, 0.9f),
      m_cachedNightColor(0.15f, 0.18f, 0.35f),
      m_sampleTimer(0),
      m_initialized(false) {
}

void EnhancedFog::reset() {
    m_initialized = false;
    m_sampleTimer = 0;
}

Color3f EnhancedFog::getBiomeFogColor(Biome* biome, float daylight) {
    daylight = Mth::clamp(daylight, 0.0f, 1.0f);

    Color3f dayCol;
    Color3f nightCol;

    if (biome == Biome::desert) {
        // warm sand
        dayCol = Color3f(0.92f, 0.82f, 0.60f);
        nightCol = Color3f(0.20f, 0.22f, 0.35f);
    } else if (biome == Biome::swampland || biome == Biome::rainForest) {
        // swamp / rainforest: grey-green
        dayCol = Color3f(0.45f, 0.55f, 0.45f);
        nightCol = Color3f(0.12f, 0.20f, 0.22f);
    } else if (biome == Biome::taiga || biome == Biome::tundra || biome == Biome::iceDesert) {
        // taiga / tundra / ice desert: pale cold blue-white
        dayCol = Color3f(0.80f, 0.85f, 0.92f);
        nightCol = Color3f(0.15f, 0.20f, 0.38f);
    } else {
        // plains / forest / seasonal forest / savanna / shrubland: light blue-grey
        dayCol = Color3f(0.70f, 0.80f, 0.90f);
        nightCol = Color3f(0.15f, 0.18f, 0.35f);
    }

    return dayCol * daylight + nightCol * (1.0f - daylight);
}

void EnhancedFog::sampleBiomeGridDayNight(Level* level, float cameraX, float cameraZ, Color3f& outDay, Color3f& outNight) {
    if (!level) {
        outDay = getBiomeFogColor(nullptr, 1.0f);
        outNight = getBiomeFogColor(nullptr, 0.0f);
        return;
    }

    Color3f sumDay(0.0f, 0.0f, 0.0f);
    Color3f sumNight(0.0f, 0.0f, 0.0f);
    const float step = 16.0f;

    for (int dz = -1; dz <= 1; ++dz) {
        for (int dx = -1; dx <= 1; ++dx) {
            float sampleX = cameraX + dx * step;
            float sampleZ = cameraZ + dz * step;

            Biome* b = level->getBiome(Mth::floor(sampleX), Mth::floor(sampleZ));
            Color3f dayCol = getBiomeFogColor(b, 1.0f);
            Color3f nightCol = getBiomeFogColor(b, 0.0f);
            sumDay = sumDay + dayCol;
            sumNight = sumNight + nightCol;
        }
    }

    outDay = sumDay * (1.0f / 9.0f);
    outNight = sumNight * (1.0f / 9.0f);
}

Color3f EnhancedFog::sampleBiomeGrid(Level* level, float cameraX, float cameraZ, float daylight) {
    Color3f dayCol, nightCol;
    sampleBiomeGridDayNight(level, cameraX, cameraZ, dayCol, nightCol);
    daylight = Mth::clamp(daylight, 0.0f, 1.0f);
    return dayCol * daylight + nightCol * (1.0f - daylight);
}

Color3f EnhancedFog::updateSmoothing(Color3f current, Color3f target, float dt, float transitionSpeed) {
    if (dt <= 0.0f) {
        return current;
    }

    float factor = 1.0f - std::exp(-transitionSpeed * dt);
    factor = Mth::clamp(factor, 0.0f, 1.0f);

    return current + (target - current) * factor;
}

void EnhancedFog::calculateFogDistances(float renderDistance, float startPct, float endPct, float& outStart, float& outEnd) {
    outStart = renderDistance * (startPct / 100.0f);
    outEnd = renderDistance * (endPct / 100.0f);
}

void EnhancedFog::update(Level* level, Mob* cameraEntity, float daylight, float dt, const FogSettings& settings) {
    if (!cameraEntity) return;

    float camX = cameraEntity->x;
    float camZ = cameraEntity->z;

    if (!m_initialized) {
        sampleBiomeGridDayNight(level, camX, camZ, m_cachedDayColor, m_cachedNightColor);
        daylight = Mth::clamp(daylight, 0.0f, 1.0f);
        m_targetColor = m_cachedDayColor * daylight + m_cachedNightColor * (1.0f - daylight);
        m_currentColor = m_targetColor;
        m_sampleTimer = 0;
        m_initialized = true;
        return;
    }

    m_sampleTimer++;
    if (m_sampleTimer >= 10) {
        m_sampleTimer = 0;
        sampleBiomeGridDayNight(level, camX, camZ, m_cachedDayColor, m_cachedNightColor);
    }

    daylight = Mth::clamp(daylight, 0.0f, 1.0f);
    m_targetColor = m_cachedDayColor * daylight + m_cachedNightColor * (1.0f - daylight);

    m_currentColor = updateSmoothing(m_currentColor, m_targetColor, dt, settings.transitionSpeed);
}
