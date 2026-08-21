#pragma once
#include "LightEngine/Fixture/ParameterConfig.h"
#include <string>

namespace LightEngine::Fixtures
{
struct FixtureTemplate
{
    std::string name = "fixture";

    FixtureTemplate() = default;
    FixtureTemplate(std::string fixName) : name(fixName) {}
    // uint16_t m_universe = 0;
    // uint8_t *m_buffer = nullptr; // -> universe buffer (non-owning)

    std::vector<ParameterConfig> parameters;
    // std::map<GDTF::Attribute, std::vector<Parameter *>> byAttribute;
    // std::vector<ColorCell> colorCells;
};

} // namespace LightEngine::Fixtures