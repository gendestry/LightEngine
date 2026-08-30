#pragma once

#include "LightEngine/Effects/Static/EffectBaseStatic.h"
#include <utility>

//
// Concrete effects. Static ones ignore time; dynamic ones animate off the
// TimeContext. Per-fixture effects use the fixture's index within the group.
// Each effect owns its group (passed by value) and contributes into the frame.
//
namespace LightEngine::Effects
{
class StaticIntensity : public EffectStatic
{
    float m_level;

public:
    StaticIntensity(float level) : m_level(level) {}

    void setLevel(float level)
    {
        m_level = level;
        markDirty();
    }

    EFFECT_CATEGORY(DIMMER);
    EFFECT_CLONE(StaticIntensity);

protected:
    void recompute(const Utils::Time::TimeContext &t,
                   const Utils::Maths::Interval &g) override;
};
} // namespace LightEngine::Effects