#include "LightEngine/Engine/Layers/Frame.h"

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
            dst.color = in.color; // latest wins (color is LTP)
        }
    }

    if (in.intensity)
    {
        if (!dst.intensity || policy == MergePolicy::LTP)
        {
            dst.intensity = in.intensity; // latest wins
        }
        else // HTP: brighter wins
        {
            dst.intensity = std::max(*dst.intensity, *in.intensity);
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
