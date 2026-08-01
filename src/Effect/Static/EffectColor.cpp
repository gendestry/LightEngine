#include "LightEngine/Effect/Static/EffectColor.h"

#include "Utils/Colors/Colors.h"

namespace LightEngine::Effects
{

void StaticColor::apply(Engine::Frame &frame, const Engine::TimeContext &,
                        const DMX::FixtureGroup &group)
{
    Engine::FixtureValues v;
    v.color = Utils::Colors::rgbToHsv(m_color); // frame stores colour as HSV
    for (uint16_t fid : group.fids())
        frame.contribute(fid, v,
                         Engine::MergePolicy::LTP); // colour: latest wins
}
} // namespace LightEngine::Effects