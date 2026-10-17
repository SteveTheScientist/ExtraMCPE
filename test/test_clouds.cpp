#include "client/renderer/CloudUtils.h"
#include <iostream>
#include <cmath>
#include <cassert>

static bool isClose(float a, float b, float tol = 1e-4f) {
    return std::fabs(a - b) < tol;
}

int main() {
    float opacity = 0.8f;
    float fadeStrength = 1.0f;
    float baseAlpha = 1.0f;

    // Default test cases:
    // 1. rel >= 2.5 gives bottom 0.8 and top 0
    float b1 = cloudAlpha(0.0f, 2.5f, baseAlpha, opacity, fadeStrength);
    float t1 = cloudAlpha(1.0f, 2.5f, baseAlpha, opacity, fadeStrength);
    float b1_far = cloudAlpha(0.0f, 10.0f, baseAlpha, opacity, fadeStrength);
    float t1_far = cloudAlpha(1.0f, 10.0f, baseAlpha, opacity, fadeStrength);

    std::cout << "rel >= 2.5 bottom: " << b1 << " (expected 0.8), top: " << t1 << " (expected 0.0)" << std::endl;
    assert(isClose(b1, 0.8f));
    assert(isClose(t1, 0.0f));
    assert(isClose(b1_far, 0.8f));
    assert(isClose(t1_far, 0.0f));

    // 2. rel <= -2.5 gives bottom 0 and top 0.8
    float b2 = cloudAlpha(0.0f, -2.5f, baseAlpha, opacity, fadeStrength);
    float t2 = cloudAlpha(1.0f, -2.5f, baseAlpha, opacity, fadeStrength);
    float b2_far = cloudAlpha(0.0f, -10.0f, baseAlpha, opacity, fadeStrength);
    float t2_far = cloudAlpha(1.0f, -10.0f, baseAlpha, opacity, fadeStrength);

    std::cout << "rel <= -2.5 bottom: " << b2 << " (expected 0.0), top: " << t2 << " (expected 0.8)" << std::endl;
    assert(isClose(b2, 0.0f));
    assert(isClose(t2, 0.8f));
    assert(isClose(b2_far, 0.0f));
    assert(isClose(t2_far, 0.8f));

    // 3. rel = 0 gives 0.3 at both bottom and top
    float b3 = cloudAlpha(0.0f, 0.0f, baseAlpha, opacity, fadeStrength);
    float t3 = cloudAlpha(1.0f, 0.0f, baseAlpha, opacity, fadeStrength);

    std::cout << "rel = 0.0 bottom: " << b3 << " (expected 0.3), top: " << t3 << " (expected 0.3)" << std::endl;
    assert(isClose(b3, 0.3f));
    assert(isClose(t3, 0.3f));

    // 4. Test Cloud Fade = 0% (fadeStrength = 0) -> plain baseAlpha * opacity everywhere
    float b_nofade = cloudAlpha(0.0f, 5.0f, baseAlpha, opacity, 0.0f);
    float t_nofade = cloudAlpha(1.0f, 5.0f, baseAlpha, opacity, 0.0f);
    assert(isClose(b_nofade, 0.8f));
    assert(isClose(t_nofade, 0.8f));

    // 5. Test baseAlpha scaling
    float b_halfBase = cloudAlpha(0.0f, 2.5f, 0.5f, opacity, fadeStrength);
    assert(isClose(b_halfBase, 0.4f));

    // 6. Test getCloudFacesDrawn
    // Camera far below (y = 50, cloudBaseY = 128, thickness = 10)
    CloudDrawnFaces f_far_below = getCloudFacesDrawn(50.0f, 128.0f, 10.0f);
    assert(f_far_below.drawTop && f_far_below.drawBottom && f_far_below.drawSides);

    // Camera just below base (y = 127.5, cloudBaseY = 128, thickness = 10)
    CloudDrawnFaces f_just_below = getCloudFacesDrawn(127.5f, 128.0f, 10.0f);
    assert(f_just_below.drawTop && f_just_below.drawBottom && f_just_below.drawSides);

    // Camera inside slab (y = 133, cloudBaseY = 128, thickness = 10)
    CloudDrawnFaces f_inside = getCloudFacesDrawn(133.0f, 128.0f, 10.0f);
    assert(f_inside.drawTop && f_inside.drawBottom && f_inside.drawSides);

    // Camera just above top (y = 138.5, cloudBaseY = 128, thickness = 10)
    CloudDrawnFaces f_just_above = getCloudFacesDrawn(138.5f, 128.0f, 10.0f);
    assert(f_just_above.drawTop && f_just_above.drawBottom && f_just_above.drawSides);

    // Camera far above (y = 200, cloudBaseY = 128, thickness = 10)
    CloudDrawnFaces f_far_above = getCloudFacesDrawn(200.0f, 128.0f, 10.0f);
    assert(f_far_above.drawTop && f_far_above.drawBottom && f_far_above.drawSides);

    // 7. Test resolveCloudModes pure function
    // Case 1: Default state (Tall Clouds ON, Beta Sky OFF, Beautiful Sky ON)
    CloudModeResolution r1 = resolveCloudModes(true, false, true);
    assert(r1.tallClouds == true && r1.betaSky == false);
    assert(r1.tallCloudsEnabled == true && r1.betaSkyEnabled == false);
    assert(r1.tallCloudsHint.empty());
    assert(r1.betaSkyHint == "Disabled (Tall Clouds & Beautiful Sky)");

    // Case 2: Turn Beta Sky ON (Tall Clouds OFF, Beautiful Sky OFF)
    CloudModeResolution r2 = resolveCloudModes(false, true, false);
    assert(r2.tallClouds == false && r2.betaSky == true);
    assert(r2.tallCloudsEnabled == false && r2.betaSkyEnabled == true);
    assert(r2.tallCloudsHint == "Disabled while Beta Sky is ON");
    assert(r2.betaSkyHint.empty());

    // Case 3: Old options file with both ON (Tall Clouds ON, Beta Sky ON, Beautiful Sky OFF)
    CloudModeResolution r3 = resolveCloudModes(true, true, false);
    assert(r3.tallClouds == false && r3.betaSky == true);
    assert(r3.tallCloudsEnabled == false && r3.betaSkyEnabled == true);

    // Case 4: Both OFF, Beautiful Sky OFF
    CloudModeResolution r4 = resolveCloudModes(false, false, false);
    assert(r4.tallClouds == false && r4.betaSky == false);
    assert(r4.tallCloudsEnabled == true && r4.betaSkyEnabled == true);
    assert(r4.tallCloudsHint.empty() && r4.betaSkyHint.empty());

    // Case 5: Both OFF, Beautiful Sky ON
    CloudModeResolution r5 = resolveCloudModes(false, false, true);
    assert(r5.tallClouds == false && r5.betaSky == false);
    assert(r5.tallCloudsEnabled == true && r5.betaSkyEnabled == false);
    assert(r5.betaSkyHint == "Disabled while Beautiful Sky is ON");

    std::cout << "All cloudAlpha, getCloudFacesDrawn, and resolveCloudModes unit tests passed successfully!" << std::endl;
    return 0;
}
