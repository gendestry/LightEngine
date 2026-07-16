#pragma once

#include "LightEngine/Engine/Frame.h"
#include "LightEngine/Engine/TimeContext.h"

//
// Layer: anything that produces values for a frame. Programmer, playback and
// effects are all just layers. update() composes them low priority -> high, so
// a higher-priority layer's LTP writes land on top of a lower one's.
//
// Concrete layers live in their own headers (e.g. ProgrammerLayer in
// Programmer.h); this file is just the interface.
//
namespace LightEngine::Engine
{

class Layer
{
public:
    virtual ~Layer() = default;

    [[nodiscard]] virtual int priority() const = 0; // sort key, low -> high
    [[nodiscard]] virtual bool enabled() const { return true; }

    // Write this layer's contribution into the frame for this tick.
    virtual void apply(Frame &frame, const TimeContext &time) = 0;
};
} // namespace LightEngine::Engine
