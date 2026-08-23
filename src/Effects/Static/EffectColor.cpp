#include "LightEngine/Effects/Static/EffectColor.h"

#include "Utils/Colors/Colors.h"

namespace LightEngine::Effects
{

void StaticColor::recompute(const Utils::Time::TimeContext &,
                            const Utils::Maths::Interval &group)
{
    // The RGB -> HSV conversion now happens once per edit instead of once per
    // fixture per frame.
    Engine::FixtureValues v;
    v.color = Utils::Colors::rgbToHsv(m_color); // frame stores colour as HSV

    m_cache.reserve(group.size());
    for (uint16_t fid : group.values())
        emit(fid, v); // colour is LTP: latest contribution wins
}
} // namespace LightEngine::Effects