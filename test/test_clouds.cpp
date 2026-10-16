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

    std::cout << "All cloudAlpha unit tests passed successfully!" << std::endl;
    return 0;
}
