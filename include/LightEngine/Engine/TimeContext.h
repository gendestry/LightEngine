#pragma once
#include <cmath>
#include <cstdint>

//
// TimeContext: the per-frame clock, threaded through update() -> Resolve().
// The Engine owns and advances it; consumers (fades, chases, effects, sequence
// playback) sample it instead of reaching for a global clock. dt is supplied by
// the caller so the render path stays deterministic and testable.
//
namespace LightEngine::Engine
{
struct TimeContext
{
    double   now   = 0.0; // seconds since engine start
    float    dt    = 0.f; // seconds since last frame
    uint64_t frame = 0;   // monotonic frame counter

    // Sawtooth phase in [0, 1) cycling at `hz` cycles per second. Effects
    // (fades, chases) animate off this so they stay frame-rate independent.
    [[nodiscard]] float phase(float hz) const
    {
        if (hz <= 0.f) return 0.f;
        const double p = now * static_cast<double>(hz);
        return static_cast<float>(p - std::floor(p));
    }
};
} // namespace LightEngine::Engine
