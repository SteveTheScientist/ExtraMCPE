#include "FogManager.h"
#include "client/Minecraft.h"
#include "client/Options.h"
#ifndef STANDALONE_SERVER
#include "client/renderer/gles.h"
#endif
#include "world/level/Level.h"
#include "world/entity/Mob.h"
#include "world/level/material/Material.h"
#include "world/level/biome/Biome.h"
#include "util/Mth.h"
#include <algorithm>
#include <cmath>

FogManager::FogManager()
    : m_currentFogColor(0.8f, 0.9f, 1.0f),
      m_currentSkyColor(0.5f, 0.7f, 1.0f),
      m_fogStart(0.0f),
      m_fogEnd(128.0f)
{
}

FogManager& FogManager::getInstance() {
    static FogManager instance;
    return instance;
}


void FogManager::update(Minecraft* mc, float partialTicks) {
    if (!mc || !mc->level || !mc->cameraTargetPlayer) return;

    Level* level = mc->level;
    Mob* player = mc->cameraTargetPlayer;

    float renderDistance = (float)(16 * 16 >> (mc->options.getIntValue(OPTIONS_VIEW_DISTANCE)));
#if defined(ANDROID)
    if (mc->isPowerVR() && mc->options.getIntValue(OPTIONS_VIEW_DISTANCE) <= 2)
        renderDistance *= 0.8f;
#endif

    bool enhancedEnabled = mc->options.getBooleanValue(OPTIONS_ENHANCED_FOG_ENABLED);
    bool betaSky = mc->options.getBooleanValue(OPTIONS_BETA_SKY);

    // If enhanced fog is disabled OR alternative sky rendering (Beta Sky) is active, fallback to legacy calculations
    if (!enhancedEnabled || betaSky) {
        computeLinearDistances(renderDistance, 1.0f, m_fogStart, m_fogEnd);
        m_fogStart = renderDistance * 0.6f;
        m_fogEnd = renderDistance;
        m_currentFogColor = level->getFogColor(partialTicks);
        m_currentSkyColor = Vec3(-1.0f, -1.0f, -1.0f);
        return;
    }

    // --- ENHANCED ATMOSPHERIC FOG CALCULATIONS ---

    // 1. Render Distance Scaling
    float distanceScale = mc->options.getProgressValue(OPTIONS_FOG_DISTANCE_SCALE);
    computeLinearDistances(renderDistance, distanceScale, m_fogStart, m_fogEnd);

    // 2. Biome & Temporal Interpolation
    int px = Mth::floor(player->x);
    int pz = Mth::floor(player->z);
    int biomeId = 1; // Default plains
    if (level->getBiomeSource()) {
        Biome* b = level->getBiome(px, pz);
        if (b) {
            if (b == Biome::desert || b == Biome::iceDesert) biomeId = 2;
            else if (b == Biome::swampland) biomeId = 6;
            else if (b == Biome::taiga) biomeId = 5;
            else if (b == Biome::forest || b == Biome::seasonalForest || b == Biome::rainForest) biomeId = 4;
            else if (b == Biome::shrubland || b == Biome::savanna) biomeId = 3;
            else if (b == Biome::tundra) biomeId = 5;
            else biomeId = 1;
        }
    }

    FogColorProfile profile = getBiomeColorProfile(biomeId);

    float timeOfDay = level->getTimeOfDay(partialTicks);
    // Diurnal cycle factor: 1 at noon (timeOfDay ~0.0/1.0 or 0.5 depending on offset, getTimeOfDay gives 0..1)
    // In MC, timeOfDay 0 is noon, 0.5 is midnight.
    float dayFactor = (Mth::cos(timeOfDay * Mth::PI * 2.0f) + 1.0f) * 0.5f;

    Vec3 baseColor = profile.nightColor * (1.0f - dayFactor) + profile.dayColor * dayFactor;

    // 3. Sunrise and Sunset Chromatic Biasing
    float sunsetBias = mc->options.getProgressValue(OPTIONS_FOG_SUNSET_BIAS);
    baseColor = calculateSunBiasedColor(baseColor, timeOfDay, sunsetBias);

    // 4. Subterranean Occlusion Attenuation
    float eyeY = player->y;
    float caveAttenuation = calculateSubterraneanAttenuation(eyeY, (float)Level::SEA_LEVEL);
    baseColor = baseColor * caveAttenuation;

    // 5. High-Altitude Atmospheric Blending
    float highAltitudeBlend = calculateHighAltitudeBlending(eyeY, (float)Level::DEPTH);
    Vec3 skyCosmicColor(0.02f, 0.02f, 0.05f);
    baseColor = baseColor * (1.0f - highAltitudeBlend) + skyCosmicColor * highAltitudeBlend;

    m_currentFogColor = baseColor;

    // 6. Horizon-Sky Color Synchronization
    if (mc->options.getBooleanValue(OPTIONS_FOG_HORIZON_SYNC)) {
        m_currentSkyColor = baseColor;
    } else {
        m_currentSkyColor = Vec3(-1.0f, -1.0f, -1.0f);
    }
}

void FogManager::setupFog(Minecraft* mc, int fogStage) {
#ifndef STANDALONE_SERVER
    if (!mc || !mc->cameraTargetPlayer) return;

    Mob* player = mc->cameraTargetPlayer;

    if (player->isUnderLiquid(Material::water)) {
        float fogBuffer[4] = { 0.02f, 0.02f, 0.2f, 1.0f };
        glFogfv(GL_FOG_COLOR, (GLfloat*)fogBuffer);
        glFogx(GL_FOG_MODE, GL_EXP);
        glFogf(GL_FOG_DENSITY, 0.1f);
        glEnable2(GL_COLOR_MATERIAL);
        return;
    } else if (player->isUnderLiquid(Material::lava)) {
        float fogBuffer[4] = { 0.6f, 0.1f, 0.0f, 1.0f };
        glFogfv(GL_FOG_COLOR, (GLfloat*)fogBuffer);
        glFogx(GL_FOG_MODE, GL_EXP);
        glFogf(GL_FOG_DENSITY, 2.0f);
        glEnable2(GL_COLOR_MATERIAL);
        return;
    }

    float fogBuffer[4] = { (float)m_currentFogColor.x, (float)m_currentFogColor.y, (float)m_currentFogColor.z, 1.0f };
    glFogfv(GL_FOG_COLOR, (GLfloat*)fogBuffer);
    glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);

    glFogx(GL_FOG_MODE, GL_LINEAR);

    if (fogStage < 0) {
        glFogf(GL_FOG_START, 0.0f);
        glFogf(GL_FOG_END, m_fogEnd);
    } else {
        glFogf(GL_FOG_START, m_fogStart);
        glFogf(GL_FOG_END, m_fogEnd);
    }

    if (mc->level && mc->level->dimension && mc->level->dimension->foggy) {
        glFogf(GL_FOG_START, 0.0f);
    }

    glEnable2(GL_COLOR_MATERIAL);
#endif
}
