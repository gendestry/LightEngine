#pragma once

#include <cmath>
#include <cstddef>
#include <memory>

#include "LightEngine/Effects/Effect.h"
#include "Utils/Colors/Colors.h"
#include "Utils/Math/Curve.h"

//
// Concrete effects. Each is a pure function of (fid, time, base) - see Effect.h.
// Per-fixture effects use the fixture's index within the group (m_index) for
// the spatial spread.
//
namespace LightEngine::Effects
{
// Sawtooth phase in [0, 1) cycling at `hz` cycles per second. LightEngine's
// TimeContext only carries `now`, so the phase is derived here.
[[nodiscard]] inline float phase(double now, float hz)
{
    if (hz <= 0.0f)
        return 0.0f;
    const double p = now * static_cast<double>(hz);
    return static_cast<float>(p - std::floor(p));
}

// A travelling intensity wave across the group, shaped by a curve.
//   bpm    - temporal rate (cycles per minute), independent of fixture count.
//   spread - spatial distribution: number of full waves across the group.
// It rides the fixture's base intensity if one was set (the wave's ceiling),
// otherwise falls back to the spec's own `level`.
class DimmerChase : public Effect
{
    std::unique_ptr<Utils::Maths::Curve> m_curve;

    // Sample the curve at a normalized position in [0, 1) (wrapping).
    static float sampleCurve(const Utils::Maths::Curve &c, float pos)
    {
        const uint32_t len = c.getLength();
        if (len == 0)
            return 0.0f;
        pos = pos - std::floor(pos);
        auto idx = static_cast<std::size_t>(pos * static_cast<float>(len));
        if (idx >= len)
            idx = len - 1;
        return c[idx];
    }

public:
    DimmerChase(Spec spec, DMX::FixtureGroup group, uint16_t resolution = 128)
        : Effect(std::move(spec), std::move(group)),
          m_curve(Utils::Maths::getCurveByType(m_spec.curve, resolution))
    {
    }

    [[nodiscard]] Sample valueFor(uint16_t fid, const Engine::TimeContext &t,
                                  const Sample &base) const override
    {
        Sample out;
        const auto it = m_index.find(fid);
        const std::size_t n = m_group.fixtures().size();
        if (it == m_index.end() || n == 0)
            return out; // not a target -> unset (does not drive this fid)

        // Temporal phase advances at the BPM rate (fixture-count independent);
        // the spatial term spreads `spread` full waves across the group.
        const float scroll = phase(t.now, m_spec.bpm / 60.0f);
        const float pos =
            static_cast<float>(it->second) / static_cast<float>(n) * m_spec.spread +
            scroll;
        const float env = sampleCurve(*m_curve, pos); // 0..1 envelope

        // Ride the base intensity (e.g. group 1's `at 100`); if none was set,
        // use the effect's own level so a bare chase still lights fixtures.
        const float ceiling = base.level.value_or(m_spec.level);
        out.level = env * ceiling;
        return out;
    }
};

// A colour that ping-pongs between colorA and colorB. With spread > 0 the blend
// position is offset per fixture, so it reads as a colour chase across the
// group rather than every fixture fading in unison.
//   bpm    - temporal rate (full A->B->A cycles per minute).
//   spread - spatial: full colour waves across the group.
// It rides the fixture's base intensity (HSV.v) if one was set, so a static
// `at 50` dims the whole fade; otherwise it runs at full.
class ColorFade : public Effect
{
    static Utils::Colors::RGB lerp(const Utils::Colors::RGB &a,
                                   const Utils::Colors::RGB &b, float t)
    {
        const auto mix = [&](uint8_t x, uint8_t y)
        {
            return static_cast<uint8_t>(x + (static_cast<float>(y) - x) * t +
                                        0.5f);
        };
        return Utils::Colors::RGB(mix(a.r, b.r), mix(a.g, b.g), mix(a.b, b.b));
    }

public:
    ColorFade(Spec spec, DMX::FixtureGroup group)
        : Effect(std::move(spec), std::move(group))
    {
    }

    [[nodiscard]] Sample valueFor(uint16_t fid, const Engine::TimeContext &t,
                                  const Sample &base) const override
    {
        Sample out;
        const auto it = m_index.find(fid);
        const std::size_t n = m_group.fixtures().size();
        if (it == m_index.end() || n == 0)
            return out; // not a target -> unset

        const float scroll = phase(t.now, m_spec.bpm / 60.0f);
        float pos = static_cast<float>(it->second) / static_cast<float>(n) *
                        m_spec.spread +
                    scroll;
        pos = pos - std::floor(pos);

        // triangle wave 0..1..0 -> a symmetric ping-pong blend A<->B.
        const float blend = pos < 0.5f ? pos * 2.0f : (1.0f - pos) * 2.0f;

        Utils::Colors::HSV hsv =
            Utils::Colors::rgbToHsv(lerp(m_spec.colorA, m_spec.colorB, blend));
        hsv.v = base.level.value_or(hsv.v); // ride the static intensity
        out.color = hsv;
        return out;
    }
};
} // namespace LightEngine::Effects
