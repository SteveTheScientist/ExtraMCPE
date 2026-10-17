#include <iostream>
#include <cassert>
#include <cmath>
#include "client/renderer/FogManager.h"
#include "util/Mth.h"

void testBiomeProfiles() {
    FogColorProfile plains = FogManager::getBiomeColorProfile(1);
    assert(plains.dayColor.x > 0.0f);
    assert(plains.nightColor.z > 0.0f);

    FogColorProfile desert = FogManager::getBiomeColorProfile(2);
    assert(desert.dayColor.x > desert.dayColor.z); // Desert should be reddish/tan

    std::cout << "testBiomeProfiles passed." << std::endl;
}

void testSunBiasedColor() {
    Vec3 base(0.5f, 0.5f, 0.5f);
    // Noon (timeOfDay = 0.0): no bias
    Vec3 noonColor = FogManager::calculateSunBiasedColor(base, 0.0f, 0.8f);
    assert(std::abs(noonColor.x - 0.5f) < 0.001f);

    // Sunset (timeOfDay = 0.75): warm bias
    Vec3 sunsetColor = FogManager::calculateSunBiasedColor(base, 0.75f, 0.8f);
    assert(sunsetColor.x > 0.5f); // Red component increases
    assert(sunsetColor.z < 0.5f); // Blue component decreases

    std::cout << "testSunBiasedColor passed." << std::endl;
}

void testSubterraneanAttenuation() {
    // Above sea level (64.0) -> factor = 1.0
    float attSurface = FogManager::calculateSubterraneanAttenuation(64.0f, 64.0f);
    assert(std::abs(attSurface - 1.0f) < 0.001f);

    // Bedrock level (0.0) -> factor = 0.0
    float attBedrock = FogManager::calculateSubterraneanAttenuation(0.0f, 64.0f);
    assert(std::abs(attBedrock - 0.0f) < 0.001f);

    // Halfway down (32.0) -> factor = (0.5)^2 = 0.25
    float attMid = FogManager::calculateSubterraneanAttenuation(32.0f, 64.0f);
    assert(std::abs(attMid - 0.25f) < 0.001f);

    std::cout << "testSubterraneanAttenuation passed." << std::endl;
}

void testHighAltitudeBlending() {
    // Below build height (128.0) -> blend = 0.0
    float blendLow = FogManager::calculateHighAltitudeBlending(64.0f, 128.0f);
    assert(std::abs(blendLow - 0.0f) < 0.001f);

    // Above build height (160.0) -> blend = 32/64 = 0.5
    float blendHigh = FogManager::calculateHighAltitudeBlending(160.0f, 128.0f);
    assert(std::abs(blendHigh - 0.5f) < 0.001f);

    std::cout << "testHighAltitudeBlending passed." << std::endl;
}

void testLinearDistances() {
    float start, end;
    FogManager::computeLinearDistances(128.0f, 1.0f, start, end);
    assert(std::abs(end - 128.0f) < 0.001f);
    assert(std::abs(start - 32.0f) < 0.001f);

    FogManager::computeLinearDistances(128.0f, 1.5f, start, end);
    assert(std::abs(end - 192.0f) < 0.001f);
    assert(std::abs(start - 48.0f) < 0.001f);

    std::cout << "testLinearDistances passed." << std::endl;
}

int main() {
    std::cout << "Running FogManager unit tests..." << std::endl;
    testBiomeProfiles();
    testSunBiasedColor();
    testSubterraneanAttenuation();
    testHighAltitudeBlending();
    testLinearDistances();
    std::cout << "All FogManager unit tests passed successfully!" << std::endl;
    return 0;
}
