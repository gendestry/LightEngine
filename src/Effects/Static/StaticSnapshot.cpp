#include "LightEngine/Effects/Static/StaticSnapshot.h"

namespace LightEngine::Effects
{
void StaticSnapshot::recompute(const Utils::Time::TimeContext &,
                               const Utils::Maths::Interval &group)
{
    // Self-masking: only emit for fixtures both in the table and in whatever
    // group this snapshot is recalled onto (may be a subset of the fixtures
    // it was stored with).
    m_cache.reserve(m_values.size());
    for (const uint64_t fid : group.values())
    {
        const auto it = m_values.find(static_cast<uint16_t>(fid));
        if (it != m_values.end())
        {
            emit(it->first, it->second);
        }
    }
}

Utils::Maths::Interval StaticSnapshot::coverage() const
{
    Utils::Maths::Interval out;
    for (const auto &[fid, values] : m_values)
    {
        out.add(fid);
    }
    return out;
}
} // namespace LightEngine::Effects
