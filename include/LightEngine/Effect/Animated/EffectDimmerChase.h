#pragma once
#include "LightEngine/Effect/EffectBase.h"

namespace LightEngine::Effects
{
class DimmerChase : public EffectAnimated
{

public:
    DimmerChase(Utils::Maths::Type type = Utils::Maths::SINUSOID,
                float bpm = 60.0f, float spread = 1.0f,
                uint16_t resolution = 128);

    void apply(Engine::Frame &frame, const Engine::TimeContext &t,
               const DMX::FixtureGroup &g) override;
    // [[nodiscard]] Spec spec() const override;
};
} // namespace LightEngine::Effects