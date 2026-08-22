#pragma once
#include "LightEngine/Fixture/Fixture.h"
#include "LightEngine/GDTF/LogicalChannel.h"
#include <initializer_list>
#include <memory>

namespace LightEngine::FixtureTools
{

class ParameterCell
{
    uint16_t m_offset = 0;
    uint16_t m_cellIndex;
    std::vector<Fixtures::ParameterConfig> parameters;

public:
    ParameterCell(uint16_t cellIndex, uint16_t relAddr) : m_cellIndex(cellIndex), m_offset(relAddr) {}

    void add8Bit(GDTF::Attribute attr)
    {
        GDTF::LogicalChannel lc;
        lc.attribute = attr;
        lc.channel = {m_offset++, GDTF::DMXChannel::Resolution::Bit8};
        lc.functions = {{"", {0, 255}, {0.f, 1.f}}};
        parameters.emplace_back(std::make_shared<const GDTF::LogicalChannel>(std::move(lc)), m_cellIndex);
    }

    void setCellIndex(uint16_t index)
    {
        m_cellIndex = index;
        for (auto &par : parameters)
        {
            par.cellIndex = index;
        }
    }

    inline uint16_t offset() const { return m_offset; }

    ParameterCell &operator<<(GDTF::Attribute attr)
    {
        add8Bit(attr);
        return *this;
    }

    inline std::vector<Fixtures::ParameterConfig> get()
    {
        return parameters;
    }
};
} // namespace LightEngine::FixtureTools