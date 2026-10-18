#ifndef UTIL_FRAME_LIMITER_H__
#define UTIL_FRAME_LIMITER_H__

#include <chrono>
#include <thread>
#include "../client/Options.h"

namespace FrameLimiter {

inline void limit(Options* options, std::chrono::steady_clock::time_point frameStart) {
    if (!options) return;

    float maxFps = options->getProgressValue(OPTIONS_LIMIT_FRAMERATE);
    if (maxFps < 260.0f && maxFps >= 10.0f) {
        auto frameEnd = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart);
        auto target = std::chrono::microseconds((long long)(1000000.0f / maxFps));
        if (elapsed < target) {
            std::this_thread::sleep_for(target - elapsed);
        }
    }
}

} // namespace FrameLimiter

#endif // UTIL_FRAME_LIMITER_H__
