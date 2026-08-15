#pragma once

#include <cstdint>
#include <memory>
#include <utility>

#include "Utils/Colors/RGB.h"
#include "Utils/Math/Curve.h"

#include "LightEngine/Effects/Effect.h"
#include "LightEngine/Effects/StaticColor.h"
#include "LightEngine/Effects/StaticIntensity.h"

//
// Concrete effects. Static ones ignore time; dynamic ones animate off the
// TimeContext. Per-fixture effects use the fixture's index within the group.
// Each effect owns its group (passed by value), contributes into the frame and
// can report its Spec.
//
namespace LightEngine::Effects
{

// Ping-pongs the group colour between two colours at `hz`.
class ColorFade : public Effect
{
    Utils::Colors::RGB m_a;
    Utils::Colors::RGB m_b;
    float m_hz;

public:
    ColorFade(DMX::FixtureGroup g, Utils::Colors::RGB a, Utils::Colors::RGB b,
              float hz = 0.5f)
        : Effect(std::move(g)), m_a(a), m_b(b), m_hz(hz)
    {
    }
    void apply(Engine::Frame &frame, const Engine::TimeContext &t) override;
    [[nodiscard]] Spec spec() const override;
};

// A travelling intensity wave across the group, shaped by a curve.
//   bpm    - temporal rate (cycles per minute), independent of fixture count.
//   spread - spatial distribution: number of full waves across the group.
class DimmerChase : public Effect
{
    std::unique_ptr<Utils::Maths::Curve> m_curve;
    Utils::Maths::Type m_type;
    float m_bpm;
    float m_spread;

public:
    DimmerChase(DMX::FixtureGroup g,
                Utils::Maths::Type type = Utils::Maths::SINUSOID,
                float bpm = 60.0f, float spread = 1.0f,
                uint16_t resolution = 128);

    void setBpm(float bpm) { m_bpm = bpm; }
    [[nodiscard]] float bpm() const { return m_bpm; }
    void setSpread(float spread) { m_spread = spread; }
    [[nodiscard]] float spread() const { return m_spread; }

    void apply(Engine::Frame &frame, const Engine::TimeContext &t) override;
    [[nodiscard]] Spec spec() const override;
};
} // namespace LightEngine::Effects
