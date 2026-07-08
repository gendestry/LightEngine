#include "LightEngine/Engine/Frame.h"

#include <algorithm>

namespace LightEngine::Engine
{
void Frame::contribute(uint16_t fid, const FixtureValues &in, MergePolicy policy)
{
    FixtureValues &dst = m_values[fid];

    if (in.color)
    {
        if (!dst.color || policy == MergePolicy::LTP)
        {
            dst.color = in.color; // latest wins
        }
        else // HTP: keep the brighter intensity, take incoming hue/sat with it
        {
            if (in.color->v > dst.color->v)
            {
                dst.color = in.color;
            }
        }
    }

    for (const auto &[attr, value] : in.generic)
    {
        auto it = dst.generic.find(attr);
        if (it == dst.generic.end() || policy == MergePolicy::LTP)
        {
            dst.generic[attr] = value;
        }
        else // HTP
        {
            it->second = std::max(it->second, value);
        }
    }
}
} // namespace LightEngine::Engine
