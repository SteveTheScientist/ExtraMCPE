#ifndef NET_MINECRAFT_CLIENT_RENDERER__EnhancedFog_H__
#define NET_MINECRAFT_CLIENT_RENDERER__EnhancedFog_H__

#include "../Options.h"

class Level;
class Biome;
class Mob;

struct Color3f {
    float r;
    float g;
    float b;

    Color3f() : r(0.0f), g(0.0f), b(0.0f) {}
    Color3f(float r, float g, float b) : r(r), g(g), b(b) {}

    Color3f operator+(const Color3f& o) const { return Color3f(r + o.r, g + o.g, b + o.b); }
    Color3f operator-(const Color3f& o) const { return Color3f(r - o.r, g - o.g, b - o.b); }
    Color3f operator*(float s) const { return Color3f(r * s, g * s, b * s); }
};

class EnhancedFog {
public:
    EnhancedFog();

    static Color3f getBiomeFogColor(Biome* biome, float daylight);
    static void sampleBiomeGridDayNight(Level* level, float cameraX, float cameraZ, Color3f& outDay, Color3f& outNight);
    static Color3f sampleBiomeGrid(Level* level, float cameraX, float cameraZ, float daylight);
    static Color3f updateSmoothing(Color3f current, Color3f target, float dt, float transitionSpeed);
    static void calculateFogDistances(float renderDistance, float startPct, float endPct, float& outStart, float& outEnd);

    static Color3f applySunsetTint(Color3f fogCol, float td, bool enabled, bool isBetaSky);
    static Color3f applyHighAltitudeBlend(Color3f fogCol, Color3f skyColLookUp, float cameraY, bool enabled, bool isBetaSky);

    void reset();
    void update(Level* level, Mob* cameraEntity, float daylight, float td, Color3f skyColLookUp, float dt, const FogSettings& settings, bool isBetaSky);

    bool isInitialized() const { return m_initialized; }
    Color3f getCurrentColor() const { return m_currentColor; }
    Color3f getTargetColor() const { return m_targetColor; }

private:
    Color3f m_currentColor;
    Color3f m_targetColor;
    Color3f m_cachedDayColor;
    Color3f m_cachedNightColor;
    int m_sampleTimer;
    bool m_initialized;
};

#endif /* NET_MINECRAFT_CLIENT_RENDERER__EnhancedFog_H__ */
