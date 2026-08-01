#pragma once

#include "LightEngine/Effect/EffectBase.h"
#include <utility>

//
// Concrete effects. Static ones ignore time; dynamic ones animate off the
// TimeContext. Per-fixture effects use the fixture's index within the group.
// Each effect owns its group (passed by value), contributes into the frame and
// can report its Spec.
//
namespace LightEngine::Effects
{
class StaticIntensity : public EffectStatic
{
    float m_level;

public:
    StaticIntensity(float level) : m_level(level) {}
    void setLevel(float level) { m_level = level; }
    void apply(Engine::Frame &frame, const Engine::TimeContext &t,
               const DMX::FixtureGroup &g) override;
    EFFECT_CATEGORY(DIMMER);
    // [[nodiscard]] Spec spec() const override;
};
} // namespace LightEngine::Effects