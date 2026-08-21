#include "LightEngine/Effects/Static/EffectIntensity.h"

namespace LightEngine::Effects
{

void StaticIntensity::recompute(const Engine::TimeContext &,
                                const Utils::Maths::Interval &g)
{
    Engine::FixtureValues v;
    v.intensity = m_level;

    m_cache.reserve(g.size());
    for (uint16_t fid : g.values())
        emit(fid, v); // absolute set (LTP)
}

} // namespace LightEngine::Effects