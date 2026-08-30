#include "LightEngine/Effects/EffectBase.h"

#include <cmath>

namespace LightEngine::Effects
{
void EffectBase::emit(uint16_t fid, const Engine::FixtureValues &values)
{
    m_cache.emplace_back(fid, values);
}

bool EffectBase::due(double now, const Utils::Maths::Interval &group) const
{
    return m_dirty || now >= m_nextDue;
}

void EffectBase::evaluate(const Utils::Time::TimeContext &t, const Utils::Maths::Interval &group)
{
    m_cache.clear();
    recompute(t, group);
    m_dirty = false;
    scheduleNext(t.now);
}

void EffectBase::replay(Engine::Frame &frame) const
{
    for (const auto &[fid, values] : m_cache)
    {
        frame.contribute(fid, values, m_policy);
    }
}

void EffectBase::scheduleNext(double now)
{
    const double interval = stepInterval();
    if (!std::isfinite(interval))
    {
        m_nextDue = NEVER; // static: only an edit brings it back
        return;
    }
    if (interval <= 0.0)
    {
        m_nextDue = now; // continuous: due again immediately
        return;
    }
    const double steps = std::floor((now - m_phaseOrigin) / interval) + 1.0;
    m_nextDue = m_phaseOrigin + steps * interval;
}

void EffectBase::syncPhase(double now)
{
    m_phaseOrigin = now;
    m_dirty = true;
}
} // namespace LightEngine::Effects
