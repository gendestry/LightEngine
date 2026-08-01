#include "LightEngine/Effect/Static/EffectIntensity.h"

namespace LightEngine::Effects
{

void StaticIntensity::apply(Engine::Frame &frame, const Engine::TimeContext &,
                            const DMX::FixtureGroup &g)
{
    Engine::FixtureValues v;
    v.intensity = m_level;
    for (uint16_t fid : g.fids())
        frame.contribute(fid, v, Engine::MergePolicy::LTP); // absolute set
}

} // namespace LightEngine::Effects