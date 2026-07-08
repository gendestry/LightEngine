#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Engine/DMXOutput.h"
#include "LightEngine/Engine/Frame.h"
#include "LightEngine/Engine/Layer.h"
#include "LightEngine/Engine/Patch.h"
#include "LightEngine/Engine/TimeContext.h"
// #include "LightEngine/Engine/Programmer.h"  // TODO: live editing layer
// #include "LightEngine/Show/Sequence.h"      // TODO: playback / cues

//
// Engine: the top-level orchestrator. Owns the Patch (universes + fixtures) and
// the named fixture groups, exposing a small API: patch -> group -> drive ->
// update. Non-tracking: update() rebuilds every universe from scratch each
// frame.
//
// Minimal version - Programmer, Sequences, Pools/Presets and sACN output are
// commented out until their subsystems exist.
//
namespace LightEngine::Engine
{
class Engine
{
    Patch m_patch;
    std::map<std::string, DMX::FixtureGroup> m_groups;
    TimeContext m_time; // advanced each update(); threaded into layers

    DMXOutput m_output;            // sACN transmit stage
    bool m_outputEnabled = false;  // set once an IP is configured

    Frame m_frame;                 // per-frame merged values (rebuilt each tick)
    ProgrammerLayer m_programmer;  // live editing layer
    std::vector<Layer *> m_layers = {&m_programmer}; // composed low -> high

    // std::map<std::string, Show::Sequence> m_sequences;   // TODO

public:
    Engine() = default;

    // ---- patching (template overload; name overload needs FixtureLibrary) ----
    std::vector<uint16_t> patch(const Fixtures::Fixture &fixture, uint16_t universe,
                                uint16_t amount,
                                std::optional<uint32_t> start = std::nullopt,
                                std::optional<uint16_t> startFID = std::nullopt)
    {
        return m_patch.patch(fixture, universe, amount, start, startFID);
    }

    // ---- groups ----
    DMX::FixtureGroup &group(const std::string &name) // get or create
    {
        auto [it, _] = m_groups.try_emplace(name, name);
        return it->second;
    }
    DMX::FixtureGroup &addToGroup(const std::string &name,
                                  const std::vector<uint16_t> &fids)
    {
        DMX::FixtureGroup &g = group(name);
        g.add(m_patch.getFixtures(fids));
        return g;
    }
    [[nodiscard]] DMX::FixtureGroup *getGroup(const std::string &name)
    {
        const auto it = m_groups.find(name);
        return it != m_groups.end() ? &it->second : nullptr;
    }

    // ---- lookup ----
    [[nodiscard]] std::shared_ptr<Fixtures::Fixture> getFixture(uint16_t fid)
    {
        return m_patch.getFixture(fid);
    }
    [[nodiscard]] DMX::Universe *getUniverse(uint16_t universe)
    {
        return m_patch.getUniverse(universe);
    }
    [[nodiscard]] Patch &patcher() { return m_patch; }

    // ---- render ----
    // Non-tracking frame: wipe every universe, then push each fixture's state
    // back into its buffer via Resolve() (color cells + generic attributes).
    // Virtual dimmers are composed inside ColorCell::Resolve(); no separate
    // pass. sACN output is a TODO. dt is the wall-clock delta since the last
    // frame (seconds); the caller owns the clock so the render path stays
    // deterministic and testable.
    void update(float dt = 0.f);

    // ---- output (sACN) ----
    // Configuring a source IP enables the transmit stage; until then update()
    // renders but sends nothing (so headless/print-only runs stay offline).
    void setIP(const std::string &ip)
    {
        m_output.setIP(ip);
        m_outputEnabled = true;
    }
    void setIP(const Utils::Network::IP &ip)
    {
        m_output.setIP(ip);
        m_outputEnabled = true;
    }
    void setSourceName(const std::string &name) { m_output.setSourceName(name); }
    [[nodiscard]] DMXOutput &output() { return m_output; }

    // ---- layers ----
    [[nodiscard]] ProgrammerLayer &programmer() { return m_programmer; }
    void addLayer(Layer *layer) { m_layers.push_back(layer); }

    [[nodiscard]] const TimeContext &time() const { return m_time; }

    [[nodiscard]] std::string describe() const { return m_patch.describe(); }
};
} // namespace LightEngine::Engine
