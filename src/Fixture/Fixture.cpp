#include "LightEngine/Fixture/Fixture.h"

#include <algorithm>
#include <print>
#include <sstream>
#include <utility>

namespace LightEngine::Fixtures
{
namespace
{
// bytes occupied by one channel (8-bit = 1, 16-bit = 2)
uint32_t channelWidth(const GDTF::DMXChannel &ch)
{
    return ch.res == GDTF::DMXChannel::Resolution::Bit16 ? 2U : 1U;
}

// is this attribute one of the color components a ColorCell groups?
bool isColorAttribute(GDTF::Attribute attr)
{
    switch (attr)
    {
    case GDTF::Attribute::COLOR_R:
    case GDTF::Attribute::COLOR_G:
    case GDTF::Attribute::COLOR_B:
    case GDTF::Attribute::COLOR_W:
        return true;
    default:
        return false;
    }
}
} // namespace

Fixture::Fixture(std::string name) : m_name(std::move(name)) {}

std::string Fixture::describe() const
{
    std::stringstream ss;
    ss << "FID: " << m_fid << " \"" << m_name << "\" [" << size
       << " bytes, ccells: " << std::to_string(m_colorCells.size()) << "]";
    return ss.str();
}

Fixture::Fixture(const Fixture &other)
    : Utils::Fragment(other), m_name(other.m_name), m_fid(other.m_fid),
      m_universe(other.m_universe), m_buffer(nullptr),
      m_parameters(other.m_parameters)
{
    for (Parameter &p : m_parameters)
    {
        p.Unbind();
    }
    // rebuild index + cells so their pointers aim at OUR parameters
    Build();
}

Fixture &Fixture::operator=(const Fixture &other)
{
    if (this != &other)
    {
        Utils::Fragment::operator=(other);
        m_name = other.m_name;
        m_fid = other.m_fid;
        m_universe = other.m_universe;
        m_buffer = nullptr;
        m_parameters = other.m_parameters;

        for (Parameter &p : m_parameters)
        {
            p.Unbind();
        }
        Build();
    }
    return *this;
}

Parameter &Fixture::Add(std::shared_ptr<const GDTF::LogicalChannel> def,
                        uint16_t cellIndex)
{
    // grow the footprint (Fragment::size) to cover this channel's slice
    const GDTF::DMXChannel &ch = def->channel;
    size = std::max<uint32_t>(size, ch.address + channelWidth(ch));

    Parameter &p = m_parameters.emplace_back(std::move(def));
    p.SetCellIndex(cellIndex);
    // std::println("Cell index {}", cellIndex);
    return p;
}

void Fixture::Build()
{
    // pointers into m_parameters are only stable after all Add()s are done,
    // so (re)build the index and cells here from scratch.
    m_byAttribute.clear();
    m_colorCells.clear();

    // how many cells? one past the highest cell index seen on a color/dimmer.
    std::size_t cellCount = 0;
    for (Parameter &p : m_parameters)
    {
        m_byAttribute[p.Attribute()].push_back(&p);
        if (isColorAttribute(p.Attribute()) || p.Is(GDTF::Attribute::DIMMER))
        {
            cellCount = std::max<std::size_t>(cellCount, p.CellIndex() + 1);
        }
    }

    m_colorCells.resize(cellCount);
    for (Parameter &p : m_parameters)
    {
        // color components and a per-cell dimmer wire into their cell.
        if (isColorAttribute(p.Attribute()) || p.Is(GDTF::Attribute::DIMMER))
        {
            m_colorCells[p.CellIndex()].SetComponent(p.Attribute(), &p);
        }
    }

    // drop cells with no color components (e.g. a lone dimmer on a non-color
    // fixture shouldn't masquerade as a color emitter).
    // m_colorCells.erase(std::remove_if(m_colorCells.begin(),
    // m_colorCells.end(),
    //                                   [](const ColorCell &c)
    //                                   { return c.IsEmpty(); }),
    //                    m_colorCells.end());
}

void Fixture::setStart(uint32_t start)
{
    this->start = start;
    // Utils::Fragment::setStart(start);
    for (Parameter &p : m_parameters)
    {
        p.SetBaseOffset(start);
    }
}

void Fixture::setBuffer(uint8_t *buffer)
{
    m_buffer = buffer;
    for (Parameter &p : m_parameters)
    {
        p.SetBuffer(buffer);
    }
}

void Fixture::Bind(uint8_t *buffer, uint32_t start)
{
    setStart(start);
    setBuffer(buffer);
}

void Fixture::Unbind()
{
    m_buffer = nullptr;
    start = 0;

    for (Parameter &p : m_parameters)
    {
        p.Unbind();
    }
}

bool Fixture::Has(GDTF::Attribute attr) const
{
    return m_byAttribute.find(attr) != m_byAttribute.end();
}

ColorCell &Fixture::Cell(std::size_t index) { return m_colorCells[index]; }

void Fixture::Resolve(const Engine::FixtureValues &values)
{
    // color cells (HSV -> RGB + virtual dimmer). Hue/sat and intensity are
    // independent contributions; touch the cells if either was set, leaving the
    // untouched component at its default (hue/sat 0 = white, intensity 0 =
    // dark). A cell at intensity 0 renders black no matter its colour.
    if (values.color || values.intensity)
    {
        for (ColorCell &cell : m_colorCells)
        {
            if (values.color)
                cell.SetHueSat(values.color->h, values.color->s);
            if (values.intensity)
                cell.SetIntensity(*values.intensity);
            cell.Resolve();
        }
    }

    // generic attributes (pan, tilt, gobo, ...)
    for (const auto &[attr, value] : values.generic)
    {
        auto it = m_byAttribute.find(attr);
        if (it == m_byAttribute.end())
        {
            continue;
        }
        for (Parameter *p : it->second)
        {
            p->Write(value);
        }
    }
}
} // namespace LightEngine::Fixtures
