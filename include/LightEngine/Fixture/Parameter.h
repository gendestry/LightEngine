#pragma once
#include <algorithm>
#include <cstdint>
#include <memory>

#include "LightEngine/GDTF/LogicalChannel.h"

//
// Parameter: a runtime view over one GDTF LogicalChannel (one attribute at one
// DMX offset). It owns no bytes and no layout - it shares a (const) definition
// (flyweight) and points into a Universe buffer. Binding (buffer + base offset)
// happens once at patch time; after that, edits just call Write().
//
// A Parameter is a single channel, NOT a cell. Cells (pixels) are groupings of
// parameters that share m_cellIndex - the GeometryReference they were expanded
// from at import time.
//
namespace LightEngine::Fixtures
{
class Parameter
{
    std::shared_ptr<const GDTF::LogicalChannel> m_def;
    uint8_t *m_bytes = nullptr;
    std::optional<uint32_t> m_baseOffset;
    uint16_t m_cellIndex = 0;

public:
    Parameter() = default;
    explicit Parameter(std::shared_ptr<const GDTF::LogicalChannel> def)
        : m_def(std::move(def))
    {
    }

    void SetDefinition(std::shared_ptr<const GDTF::LogicalChannel> def)
    {
        m_def = std::move(def);
    }

    void SetBuffer(uint8_t *bytes) { m_bytes = bytes; }
    void SetBaseOffset(std::optional<uint32_t> offset)
    {
        m_baseOffset = offset;
    }
    void SetCellIndex(uint16_t cell) { m_cellIndex = cell; }

    // ---- queries ----
    const std::shared_ptr<const GDTF::LogicalChannel> &Definition() const
    {
        return m_def;
    }

    GDTF::Attribute Attribute() const { return m_def->attribute; }
    uint16_t CellIndex() const { return m_cellIndex; }

    bool Is(GDTF::Attribute attr) const
    {
        return m_def && m_def->attribute == attr;
    }

    bool Bound() const { return m_def && m_bytes; }

    void Unbind()
    {
        SetBuffer(nullptr);
        SetBaseOffset(std::nullopt);
    }

    void Write(float value)
    {
        if (!Bound())
        {
            return;
        }

        const GDTF::DMXChannel &ch = m_def->channel;
        for (const auto &fn : m_def->functions)
        {
            if (value >= fn.range_physical.min &&
                value <= fn.range_physical.max)
            {
                float denom = fn.range_physical.max - fn.range_physical.min;
                float norm = (denom > 0.f)
                                 ? (value - fn.range_physical.min) / denom
                                 : 0.f;
                float span = float(fn.range_dmx.to - fn.range_dmx.from);
                uint16_t dmx = fn.range_dmx.from +
                               static_cast<uint16_t>(norm * span + 0.5f);
                WriteRaw(ch, std::min(dmx, ch.MaxDMX()));
                return;
            }
        }
    }

private:
    void WriteRaw(const GDTF::DMXChannel &ch, uint16_t dmx)
    {
        uint32_t addr = m_baseOffset.value() + ch.address;
        if (ch.res == GDTF::DMXChannel::Resolution::Bit8)
        {
            m_bytes[addr] = static_cast<uint8_t>(dmx);
        }
        else
        {
            m_bytes[addr] = static_cast<uint8_t>(dmx >> 8);
            m_bytes[addr + 1] = static_cast<uint8_t>(dmx & 0xFF);
        }
    }
};
} // namespace LightEngine::Fixtures
