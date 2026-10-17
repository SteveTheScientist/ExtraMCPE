#ifndef NET_MINECRAFT_CLIENT_RENDERER__FogManager_H__
#define NET_MINECRAFT_CLIENT_RENDERER__FogManager_H__

#include "world/phys/Vec3.h"
#include <algorithm>
#include <cmath>

class Minecraft;

struct FogColorProfile {
    Vec3 dayColor;
    Vec3 nightColor;
};

class FogManager {
public:
    static FogManager& getInstance();

    void update(Minecraft* mc, float partialTicks);
    void setupFog(Minecraft* mc, int fogStage);

    Vec3 getCalculatedFogColor() const { return m_currentFogColor; }
    Vec3 getCalculatedSkyColor() const { return m_currentSkyColor; }

    float getFogStart() const { return m_fogStart; }
    float getFogEnd() const { return m_fogEnd; }

    // Pure math helpers
    static inline FogColorProfile getBiomeColorProfile(int biomeId) {
        switch (biomeId) {
        case 0: // Ocean
            return { Vec3(0.090f, 0.345f, 0.722f), Vec3(0.020f, 0.075f, 0.212f) };
        case 2: // Desert
            return { Vec3(0.851f, 0.776f, 0.647f), Vec3(0.165f, 0.122f, 0.094f) };
        case 3: // Mountains
            return { Vec3(0.486f, 0.635f, 0.733f), Vec3(0.059f, 0.102f, 0.149f) };
        case 4: // Forest
            return { Vec3(0.490f, 0.702f, 0.541f), Vec3(0.055f, 0.133f, 0.094f) };
        case 5: // Taiga
            return { Vec3(0.506f, 0.639f, 0.647f), Vec3(0.063f, 0.110f, 0.141f) };
        case 6: // Swamp
            return { Vec3(0.306f, 0.376f, 0.263f), Vec3(0.039f, 0.071f, 0.039f) };
        case 7: // River
            return { Vec3(0.200f, 0.471f, 0.733f), Vec3(0.031f, 0.094f, 0.165f) };
        case 1: // Plains
        default:
            return { Vec3(0.569f, 0.761f, 1.000f), Vec3(0.047f, 0.106f, 0.220f) };
        }
    }

    static inline Vec3 calculateSunBiasedColor(const Vec3& baseColor, float timeOfDay, float sunriseSunsetBias) {
        float sunriseDist = std::abs(timeOfDay - 0.25f);
        float sunsetDist = std::abs(timeOfDay - 0.75f);
        float sunFactor = std::max(0.0f, 1.0f - std::min(sunriseDist, sunsetDist) / 0.08f);

        if (sunFactor <= 0.0f || sunriseSunsetBias <= 0.0f) {
            return baseColor;
        }

        Vec3 warmColor(1.00f, 0.45f, 0.15f);
        float blendAmount = sunFactor * sunriseSunsetBias;
        return baseColor * (1.0f - blendAmount) + warmColor * blendAmount;
    }

    static inline float calculateSubterraneanAttenuation(float eyeY, float seaLevel = 64.0f) {
        if (eyeY >= seaLevel) {
            return 1.0f;
        }
        float factor = eyeY / seaLevel;
        if (factor < 0.0f) factor = 0.0f;
        return factor * factor;
    }

    static inline float calculateHighAltitudeBlending(float eyeY, float maxBuildHeight = 128.0f) {
        if (eyeY <= maxBuildHeight) {
            return 0.0f;
        }
        float altitudeOver = eyeY - maxBuildHeight;
        float blend = altitudeOver / 64.0f;
        return (blend < 0.0f) ? 0.0f : ((blend > 1.0f) ? 1.0f : blend);
    }

    static inline void computeLinearDistances(float renderDistance, float distanceScale, float& outStart, float& outEnd) {
        float effectiveMax = renderDistance * distanceScale;
        outStart = effectiveMax * 0.25f;
        outEnd = effectiveMax;
    }

private:
    FogManager();

    Vec3 m_currentFogColor;
    Vec3 m_currentSkyColor;
    float m_fogStart;
    float m_fogEnd;
};

#endif /* NET_MINECRAFT_CLIENT_RENDERER__FogManager_H__ */
