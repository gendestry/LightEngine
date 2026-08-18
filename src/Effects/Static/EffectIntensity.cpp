#include "LightEngine/Effects/Static/EffectIntensity.h"

namespace LightEngine::Effects
{

void StaticIntensity::recompute(const Engine::TimeContext &,
                                const DMX::FixtureGroup &g)
{
    Engine::FixtureValues v;
    v.intensity = m_level;

    m_cache.reserve(g.size());
    for (uint16_t fid : g.fids())
        emit(fid, v); // absolute set (LTP)
}

} // namespace LightEngine::Effects