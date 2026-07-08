#pragma once
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
};
} // namespace LightEngine::Engine
