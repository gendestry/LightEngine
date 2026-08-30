#include "LightEngine/Effects/Animated/EffectDimmerChase.h"

#include <cmath>

namespace LightEngine::Effects
{
namespace
{
// Sample a curve at a wrapped, normalised position. The curve holds
// `resolution` samples, so this is where the chase's output gets quantised into
// discrete steps - which is exactly what stepInterval() schedules against.
float sampleCurve(const Utils::Maths::Curve &c, float pos)
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
} // namespace

DimmerChase::DimmerChase(Utils::Maths::Type type, float bpm, float spread,
                         uint16_t resolution)
    : m_type(type)
{
    m_curve = Utils::Maths::getCurveByType(type, resolution);
    m_bpm = bpm;
    m_spread = spread;
    // One curve cycle per beat, sampled `resolution` times -> the effect has
    // new output `resolution` times per beat and no more. EffectAnimated turns
    // this into stepInterval() = (60/bpm)/resolution.
    m_steps = resolution;
    // A dimmer effect merges HTP: it lifts levels, it does not cut them.
    m_policy = Engine::MergePolicy::HTP;
}

void DimmerChase::setCurve(Utils::Maths::Type type)
{
    m_type = type;
    m_curve = Utils::Maths::getCurveByType(type, m_steps);
    markDirty();
}

std::shared_ptr<EffectBase> DimmerChase::clone() const
{
    // Rebuilds the curve rather than copying m_curve (unique_ptr, non-copyable).
    // m_steps doubles as the curve resolution - see the constructor.
    auto c = std::make_shared<DimmerChase>(m_type, m_bpm, m_spread, m_steps);
    c->setEnabled(enabled());
    c->markDirty();
    return c;
}

void DimmerChase::recompute(const Utils::Time::TimeContext &t,
                            const Utils::Maths::Interval &g)
{
    const auto &fids = g.values();
    const std::size_t n = fids.size();
    if (n == 0 || !m_curve)
        return;

    // Temporal phase advances at the BPM rate (independent of fixture
    // count),
    // measured from this effect's own grid anchor so a tap-sync or rate
    // change
    // re-phases it cleanly. The spatial term spreads m_spread full waves
    // across
    // the selection.
    const double cycles = (t.now - m_phaseOrigin) * (m_bpm / 60.0);
    const auto scroll = static_cast<float>(cycles - std::floor(cycles));

    m_cache.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        const float pos =
            static_cast<float>(i) / static_cast<float>(n) * m_spread + scroll;
        Engine::FixtureValues v;
        v.intensity = sampleCurve(*m_curve, pos);
        emit(fids[i], v);
    }
}
} // namespace LightEngine::Effects
