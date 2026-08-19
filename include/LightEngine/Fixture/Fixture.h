#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "LightEngine/Engine/Layers/Frame.h"
#include "LightEngine/Fixture/ColorCell.h"
#include "LightEngine/Fixture/FixtureConfig.h"
#include "LightEngine/Fixture/Parameter.h"
#include "LightEngine/GDTF/LogicalChannel.h"
#include "Utils/Storage/FragmentedStorage.h"

//
// Fixture: one patched light. It is a Utils::Fragment, so a Universe
// (FragmentedStorage) can pack it: the storage assigns its start via
// setStart(), the Universe wires its buffer via setBuffer(); both are
// propagated down to every parameter.
//
// Holds ALL parameters (color, dimmer, pan, tilt, gobo, ...) flat, an attribute
// index for direct access, and color cells (views over the color parameters,
// grouped per emitter/pixel).
//
// Build order: Add() every parameter, then Build() to construct the index and
// color cells (pointers into the now-stable parameter vector). Placement
// (setStart/setBuffer) happens when the fixture is added to a Universe.
//
namespace LightEngine::Fixtures
{
class Fixture : public Utils::Fragment
{
    std::shared_ptr<FixtureTemplate> m_config;
    std::string m_name = "fixture";
    uint16_t m_fid = 0;
    uint16_t m_universe = 0;
    uint8_t *m_buffer = nullptr; // -> universe buffer (non-owning)

    std::vector<Parameter> m_parameters;
    std::map<GDTF::Attribute, std::vector<Parameter *>> m_byAttribute;
    std::vector<ColorCell> m_colorCells;

public:
    Fixture() = default;
    explicit Fixture(std::string name);

    // Deep copy: parameters are copied, then the index + cells are rebuilt so
    // their pointers aim at THIS instance's parameters (not the source's).
    Fixture(const Fixture &other);
    Fixture &operator=(const Fixture &other);

    explicit Fixture(std::shared_ptr<FixtureTemplate> tmpl)
        : m_config(std::move(tmpl))
    {
        for (const auto &config : m_config->parameters)
        {
            Add(config.definition, config.cellIndex);
        }

        Build();
    }

    // ---- build ----
    // Append a parameter from a shared definition, tagged with its
    // emitter/cell.
    Parameter &Add(std::shared_ptr<const GDTF::LogicalChannel> def,
                   uint16_t cellIndex = 0);
    // Build the attribute index + color cells. Call once after all Add()s.
    void Build();

    // ---- Fragment hooks: propagate placement to the parameters ----
    void setStart(uint32_t start) override;
    void setBuffer(uint8_t *buffer);
    // Convenience for standalone use (universe does the two hooks itself).
    void Bind(uint8_t *buffer, uint32_t start);
    void Unbind();

    // ---- generic attribute access (pan, tilt, gobo, ...) ----
    bool Has(GDTF::Attribute attr) const;

    // ---- color path (per emitter, HSV + virtual dimmer) ----
    ColorCell &Cell(std::size_t index);
    std::vector<ColorCell> &ColorCells() { return m_colorCells; }
    std::size_t CellCount() const { return m_colorCells.size(); }

    // ---- frame resolve: write this frame's merged values into the buffer ----
    // Fixtures are stateless sinks: they keep nothing between frames, they just
    // render whatever the compositor hands them.
    void Resolve(const Engine::FixtureValues &values);

    // ---- identity ----
    // Shadows Utils::Fragment::describe(). fragmentsToString() calls
    // describe() on a shared_ptr<Fixture> (static type), so this resolves
    // by name-hiding - no virtual needed. Prints the real FID, not the
    // per-universe fragment id.
    [[nodiscard]] std::string describe() const;

    const std::string &Name() const { return m_name; }
    uint16_t Fid() const { return m_fid; }
    void SetFid(uint16_t fid) { m_fid = fid; }
    uint16_t Universe() const { return m_universe; }
    void SetUniverse(uint16_t universe) { m_universe = universe; }
    uint32_t Footprint() const { return size; }
    const std::vector<Parameter> &Parameters() const { return m_parameters; }
    const std::map<GDTF::Attribute, std::vector<Parameter *>> &
    ByAttribute() const
    {
        return m_byAttribute;
    }
};
} // namespace LightEngine::Fixtures
