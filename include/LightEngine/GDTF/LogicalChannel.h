#pragma once
#include "DMXChannel.h"
#include "Ranges.h"
#include <vector>

namespace LightEngine::GDTF
{
enum class Attribute
{
    DIMMER,
    VDIMMER,
    COLOR_R,
    COLOR_G,
    COLOR_B,
    COLOR_W,
};
struct LogicalChannel
{
    Attribute attribute;
    DMXChannel channel;
    std::vector<ChannelFunction> functions;
};
} // namespace LightEngine::GDTF