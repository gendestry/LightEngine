#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "LightEngine/Engine/Layers/Frame.h"
#include "LightEngine/Fixture/ColorCell.h"
#include "LightEngine/Fixture/Parameter.h"
#include "LightEngine/GDTF/LogicalChannel.h"
#include "Utils/Storage/FragmentedStorage.h"

namespace LightEngine::Fixtures
{
struct FixtureTemplate
{
    std::string name = "fixture";
    // uint16_t fid = 0;
    // uint16_t uid = 0;

    FixtureTemplate() = default;
    FixtureTemplate(std::string fixName) : name(fixName) {}
    // uint16_t m_universe = 0;
    // uint8_t *m_buffer = nullptr; // -> universe buffer (non-owning)

    std::vector<ParameterConfig> parameters;
    // std::map<GDTF::Attribute, std::vector<Parameter *>> byAttribute;
    // std::vector<ColorCell> colorCells;
};

} // namespace LightEngine::Fixtures