#pragma once

#include "LightEngine/Effects/Static/EffectBaseStatic.h"
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
    StaticIntensity(
        const std::vector<std::pair<uint16_t, Engine::FixtureValues>> &vals)
        : EffectStatic(vals)
    {
    }

    void setLevel(float level)
    {
        m_level = level;
        markDirty();
    }
    EFFECT_CATEGORY(DIMMER);

protected:
    void recompute(const Engine::TimeContext &t,
                   const DMX::FixtureGroup &g) override;
    // [[nodiscard]] Spec spec() const override;
};
} // namespace LightEngine::Effects