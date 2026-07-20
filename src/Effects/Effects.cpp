#include "LightEngine/Effects/Effects.h"

#include <cmath>
#include <cstddef>

#include "Utils/Colors/Colors.h"

#include "LightEngine/Fixture/Fixture.h"

namespace LightEngine::Effects
{
namespace
{
    Utils::Colors::RGB lerp(const Utils::Colors::RGB& a, const Utils::Colors::RGB& b, float t)
    {
        auto mix = [t](uint8_t x, uint8_t y) {
            return static_cast<uint8_t>(x + (static_cast<float>(y) - static_cast<float>(x)) * t);
        };
        return {mix(a.r, b.r), mix(a.g, b.g), mix(a.b, b.b)};
    }

    // Sample a curve at a normalized position in [0, 1) (wrapping).
    float sampleCurve(const Utils::Maths::Curve& c, float pos)
    {
        const uint32_t len = c.getLength();
        if (len == 0) return 0.0f;
        pos = pos - std::floor(pos);
        auto idx = static_cast<std::size_t>(pos * static_cast<float>(len));
        if (idx >= len) idx = len - 1;
        return c[idx];
    }
}

// ---- ColorEffect ----
void ColorEffect::apply(Engine::Frame& frame, const Engine::TimeContext&)
{
    Engine::FixtureValues v;
    v.color = Utils::Colors::rgbToHsv(m_color);   // frame stores colour as HSV
    for (uint16_t fid : m_group.fids())
        frame.contribute(fid, v, Engine::MergePolicy::LTP);   // colour: latest wins
}

Spec ColorEffect::spec() const
{
    Spec s;
    s.kind   = Kind::ColorEffect;
    s.fids   = targetFids();
    s.colorA = m_color;
    return s;
}

// ---- DimmerEffect ----
void DimmerEffect::apply(Engine::Frame& frame, const Engine::TimeContext&)
{
    Engine::FixtureValues v;
    v.intensity = m_level;
    for (uint16_t fid : m_group.fids())
        frame.contribute(fid, v, Engine::MergePolicy::LTP);   // absolute set
}

Spec DimmerEffect::spec() const
{
    Spec s;
    s.kind  = Kind::DimmerEffect;
    s.fids  = targetFids();
    s.level = m_level;
    return s;
}

// ---- ColorFade ----
void ColorFade::apply(Engine::Frame& frame, const Engine::TimeContext& t)
{
    const float p = t.phase(m_hz);
    const float ping = p < 0.5f ? p * 2.0f : (1.0f - p) * 2.0f;   // 0..1..0
    Engine::FixtureValues v;
    v.color = Utils::Colors::rgbToHsv(lerp(m_a, m_b, ping));
    for (uint16_t fid : m_group.fids())
        frame.contribute(fid, v, Engine::MergePolicy::LTP);
}

Spec ColorFade::spec() const
{
    Spec s;
    s.kind   = Kind::ColorFade;
    s.fids   = targetFids();
    s.colorA = m_a;
    s.colorB = m_b;
    s.bpm    = m_hz * 60.0f;
    return s;
}

// ---- DimmerChase ----
DimmerChase::DimmerChase(DMX::FixtureGroup g, Utils::Maths::Type type, float bpm, float spread, uint16_t resolution)
    : Effect(std::move(g)),
      m_curve(Utils::Maths::getCurveByType(type, resolution)),
      m_type(type), m_bpm(bpm), m_spread(spread) {}

void DimmerChase::apply(Engine::Frame& frame, const Engine::TimeContext& t)
{
    const auto& fixtures = m_group.fixtures();
    const std::size_t n = fixtures.size();
    if (n == 0) return;

    // Temporal phase advances at the BPM rate (fixture-count independent);
    // the spatial term spreads `m_spread` full waves across the group.
    const float scroll = t.phase(m_bpm / 60.0f);
    for (std::size_t i = 0; i < n; ++i)
    {
        const float pos = static_cast<float>(i) / static_cast<float>(n) * m_spread + scroll;
        Engine::FixtureValues v;
        v.intensity = sampleCurve(*m_curve, pos);
        frame.contribute(fixtures[i]->Fid(), v, Engine::MergePolicy::HTP);
    }
}

Spec DimmerChase::spec() const
{
    Spec s;
    s.kind   = Kind::DimmerChase;
    s.fids   = targetFids();
    s.curve  = m_type;
    s.bpm    = m_bpm;
    s.spread = m_spread;
    return s;
}
} // namespace LightEngine::Effects
