#include "EnhancedFog.h"
#include "../../world/level/Level.h"
#include "../../world/level/biome/Biome.h"
#include "../../world/entity/Mob.h"
#include "../../util/Mth.h"
#include <cmath>
#include <algorithm>

EnhancedFog::EnhancedFog()
    : m_currentBaseColor(0.8f, 0.85f, 0.9f),
      m_currentColor(0.8f, 0.85f, 0.9f),
      m_cachedDayColor(0.8f, 0.85f, 0.9f),
      m_cachedNightColor(0.15f, 0.18f, 0.35f),
      m_undergroundFactor(0.0f),
      m_sampleTimer(0),
      m_initialized(false) {
}

void EnhancedFog::reset() {
    m_initialized = false;
    m_undergroundFactor = 0.0f;
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

Color3f EnhancedFog::applySunsetTint(Color3f fogCol, float td, bool enabled) {
    if (!enabled) {
        return fogCol;
    }

    float cosVal = Mth::cos(td * Mth::PI * 2.0f);
    float factor = 1.0f - std::abs(cosVal); // 1.0 at sunrise/sunset, 0.0 at midday/midnight
    factor = Mth::clamp(factor, 0.0f, 1.0f);

    Color3f warmTint(1.0f, 0.45f, 0.15f);
    float tintAmount = factor * 0.35f;

    return fogCol * (1.0f - tintAmount) + warmTint * tintAmount;
}

Color3f EnhancedFog::applyHighAltitudeBlend(Color3f fogCol, Color3f skyColLookUp, float cameraY, bool enabled) {
    if (!enabled) {
        return fogCol;
    }

    // Start 20 blocks above sea level (64 + 20 = 84), max at 80 blocks above (64 + 80 = 144)
    float startY = 84.0f;
    float maxY = 144.0f;

    float altFactor = Mth::clamp((cameraY - startY) / (maxY - startY), 0.0f, 1.0f);
    float blendWeight = altFactor * 0.70f; // max strength 70%

    return fogCol * (1.0f - blendWeight) + skyColLookUp * blendWeight;
}

float EnhancedFog::calculateUndergroundFactor(float cameraY, float surfaceY, bool canSeeSky, bool enabled) {
    if (!enabled || canSeeSky) {
        return 0.0f;
    }

    // Well below surface with no sky access
    float depthBelowSurface = surfaceY - cameraY;
    if (depthBelowSurface <= 2.0f) {
        return 0.0f;
    }

    // Smoothly scale factor from 0.0 at depth=2 to 1.0 at depth=20
    float factor = (depthBelowSurface - 2.0f) / 18.0f;
    return Mth::clamp(factor, 0.0f, 1.0f);
}

void EnhancedFog::update(Level* level, Mob* cameraEntity, float daylight, float td, Color3f skyColLookUp, float dt, const FogSettings& settings, float surfaceY, bool canSeeSky) {
    if (!cameraEntity) return;

    float camX = cameraEntity->x;
    float camY = cameraEntity->y;
    float camZ = cameraEntity->z;

    daylight = Mth::clamp(daylight, 0.0f, 1.0f);

    if (!m_initialized) {
        sampleBiomeGridDayNight(level, camX, camZ, m_cachedDayColor, m_cachedNightColor);
        m_currentBaseColor = m_cachedDayColor * daylight + m_cachedNightColor * (1.0f - daylight);
        m_sampleTimer = 0;
        m_initialized = true;
    } else {
        m_sampleTimer++;
        if (m_sampleTimer >= 10) {
            m_sampleTimer = 0;
            sampleBiomeGridDayNight(level, camX, camZ, m_cachedDayColor, m_cachedNightColor);
        }

        Color3f targetBase = m_cachedDayColor * daylight + m_cachedNightColor * (1.0f - daylight);
        m_currentBaseColor = updateSmoothing(m_currentBaseColor, targetBase, dt, settings.transitionSpeed);
    }

    Color3f colWithTint = applySunsetTint(m_currentBaseColor, td, settings.sunsetTint);
    Color3f colWithAlt = applyHighAltitudeBlend(colWithTint, skyColLookUp, camY, settings.highAltitudeBlend);

    float targetUgFactor = calculateUndergroundFactor(camY, surfaceY, canSeeSky, settings.undergroundFog);
    if (dt > 0.0f) {
        float easeSpeed = settings.transitionSpeed;
        float factor = 1.0f - std::exp(-easeSpeed * dt);
        factor = Mth::clamp(factor, 0.0f, 1.0f);
        m_undergroundFactor = m_undergroundFactor + (targetUgFactor - m_undergroundFactor) * factor;
    } else {
        m_undergroundFactor = targetUgFactor;
    }

    Color3f nearBlack(0.02f, 0.02f, 0.03f);
    m_currentColor = colWithAlt * (1.0f - m_undergroundFactor * 0.90f) + nearBlack * (m_undergroundFactor * 0.90f);
}
