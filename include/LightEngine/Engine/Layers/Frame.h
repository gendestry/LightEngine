#pragma once
#include <cstdint>
#include <map>
#include <optional>

#include "LightEngine/GDTF/LogicalChannel.h"
#include "Utils/Colors/HSV.h"

//
// Frame: the per-frame value buffer that layers write into. It replaces the old
// "state lives on the fixture" model: each frame is built fresh, layers merge
// their contributions in priority order, then the merged values are resolved to
// DMX. Keyed by Fixture ID (FID).
//
namespace LightEngine::Engine
{
// How overlapping contributions combine, per lighting-console convention.
enum class MergePolicy
{
    HTP, // Highest-Takes-Precedence - intensity (max wins)
    LTP, // Latest/priority-Takes-Precedence - color, position (last wins)
};

// One fixture's contribution within a single frame. Everything optional: a
// layer only fills what it controls, leaving the rest for lower layers.
struct FixtureValues
{
    std::optional<Utils::Colors::HSV> color;   // hue/sat (V unused here)
    std::optional<float> intensity;            // dimmer, 0..1 (separate from V)
    std::map<GDTF::Attribute, float> generic;  // pan, tilt, gobo, ...
};

class Frame
{
    std::map<uint16_t, FixtureValues> m_values;

public:
    void clear() { m_values.clear(); }

    // Merge one layer's contribution for a fixture into the frame. `policy`
    // decides what wins when two layers touch the same value.
    void contribute(uint16_t fid, const FixtureValues &in, MergePolicy policy);

    [[nodiscard]] const FixtureValues *get(uint16_t fid) const
    {
        const auto it = m_values.find(fid);
        return it != m_values.end() ? &it->second : nullptr;
    }
    [[nodiscard]] const std::map<uint16_t, FixtureValues> &all() const
    {
        return m_values;
    }
};
} // namespace LightEngine::Engine
