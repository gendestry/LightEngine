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
#include "LightEngine/Engine/Pools/Stored.h"
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
    TimeContext m_time; // advanced each update(); threaded into layers

    DMXOutput m_output;           // sACN transmit stage
    bool m_outputEnabled = false; // set once an IP is configured

    Frame m_frame;                // per-frame merged values (rebuilt each tick)
    ProgrammerLayer m_programmer; // live editing layer
    std::vector<Layer *> m_layers = {&m_programmer}; // composed low -> high

    Stored m_stored; // all object pools (groups, presets, cues...) live here

public:
    Engine() : m_programmer(m_patch) {} // m_patch is declared first -> safe

    // ---- patching (template overload; name overload needs FixtureLibrary)
    // ----
    std::vector<uint16_t> patch(const Fixtures::Fixture &fixture,
                                uint16_t universe, uint16_t amount,
                                std::optional<uint32_t> start = std::nullopt,
                                std::optional<uint16_t> startFID = std::nullopt)
    {
        return m_patch.patch(fixture, universe, amount, start, startFID);
    }

    // ---- stored pools ----
    [[nodiscard]] Stored &stored() { return m_stored; }
    [[nodiscard]] const Stored &stored() const { return m_stored; }

    // ---- programmer ----
    // Console-style Clear: wipe the programmer's values and selection so it
    // stops contributing to the frame.
    void clear() { m_programmer.clearAll(); }

    // ---- programmer selection & store ----
    // Select a stored group into the programmer (replace).
    void selectGroup(uint32_t num)
    {
        if (auto grp = m_stored.groups().get(num))
            m_programmer.select(grp->fixtureGroup());
    }
    // Snapshot the current selection into the group pool at `num` / next free.
    Pools::Group &storeGroup(uint32_t num)
    {
        return m_stored.groups().emplaceAt(num,
                                           m_programmer.selection().fixtures());
    }
    Pools::Group &storeGroup()
    {
        return m_stored.groups().emplace(m_programmer.selection().fixtures());
    }

    // ---- color presets ----
    // Capture the current programmer color (h,s) per selected fixture into a
    // color preset. Fixtures with no color edit are skipped, so the preset's
    // keys are exactly the fixtures it applies to.
    Pools::ColorPreset &storeColorPreset(uint32_t num)
    {
        auto preset = std::make_shared<Pools::ColorPreset>();
        const auto &edits = m_programmer.edits();
        for (const auto &f : m_programmer.selection().fixtures())
        {
            const auto it = edits.find(f->Fid());
            if (it != edits.end() && it->second.color)
                preset->set(f->Fid(),
                            {it->second.color->h, it->second.color->s});
        }
        return m_stored.colorPresets().store(num, std::move(preset));
    }
    // Recall a color preset onto the current selection (only fixtures the
    // preset stored are touched; the rest are left as-is).
    void recallColorPreset(uint32_t num)
    {
        if (auto preset = m_stored.colorPresets().get(num))
            preset->recall(m_programmer, m_programmer.selection());
    }

    // ---- dimmer presets ----
    // Capture the current programmer intensity (HSV.v) per selected fixture.
    Pools::DimmerPreset &storeDimmerPreset(uint32_t num)
    {
        auto preset = std::make_shared<Pools::DimmerPreset>();
        const auto &edits = m_programmer.edits();
        for (const auto &f : m_programmer.selection().fixtures())
        {
            const auto it = edits.find(f->Fid());
            if (it != edits.end() && it->second.color)
                preset->set(f->Fid(), it->second.color->v);
        }
        return m_stored.dimmerPresets().store(num, std::move(preset));
    }
    // Recall a dimmer preset onto the current selection (stored fixtures only).
    void recallDimmerPreset(uint32_t num)
    {
        if (auto preset = m_stored.dimmerPresets().get(num))
            preset->recall(m_programmer, m_programmer.selection());
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
    void setSourceName(const std::string &name)
    {
        m_output.setSourceName(name);
    }
    [[nodiscard]] DMXOutput &output() { return m_output; }

    // ---- layers ----
    [[nodiscard]] ProgrammerLayer &programmer() { return m_programmer; }
    void addLayer(Layer *layer) { m_layers.push_back(layer); }

    [[nodiscard]] const TimeContext &time() const { return m_time; }

    [[nodiscard]] std::string describe() const { return m_patch.describe(); }
};
} // namespace LightEngine::Engine
