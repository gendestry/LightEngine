#include "LightEngine/Effects/StaticColor.h"

namespace LightEngine::Effects
{

void StaticColor::apply(Engine::Frame &frame, const Engine::TimeContext &)
{
    Engine::FixtureValues v;
    v.color = Utils::Colors::rgbToHsv(m_color); // frame stores colour as HSV
    for (uint16_t fid : m_group.fids())
        frame.contribute(fid, v,
                         Engine::MergePolicy::LTP); // colour: latest wins
}

Spec StaticColor::spec() const
{
    Spec s;
    s.kind = Kind::ColorEffect;
    s.fids = targetFids();
    s.colorA = m_color;
    return s;
}
} // namespace LightEngine::Effects