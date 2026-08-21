#pragma once
#include <cstdint>
#include <memory>

#include "LightEngine/GDTF/LogicalChannel.h"

namespace LightEngine::Fixtures
{
struct ParameterConfig
{
    std::shared_ptr<const GDTF::LogicalChannel> definition;
    uint16_t cellIndex = 0;
};

} // namespace LightEngine::Fixtures