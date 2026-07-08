#pragma once
#include <cstdint>
#include <string>

namespace LightEngine::GDTF
{
struct DMXRange
{
    uint32_t from;
    uint32_t to;

    bool Contains(uint32_t value) const
    {
        return (value >= from) && (value <= to);
    }

    float Normalize(uint32_t value) const
    {
        return float(value - from) / float(to - from);
    }
};

struct PhysicalRange
{
    float min = 0.f;
    float max = 1.f;

    float Normalize(float value) { return (value - min) / (max - min); }

    float FromNormalized(float value) { return min + value * (max - min); }
};

struct ChannelFunction
{
    std::string name;
    DMXRange range_dmx;
    PhysicalRange range_physical;

    float Evaluate(uint32_t value)
    {
        float norm = range_dmx.Normalize(value);
        return range_physical.FromNormalized(norm);
    }
};
} // namespace LightEngine::GDTF