#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "LightEngine/Engine/DMXOutput.h"
#include "LightEngine/Engine/Frame.h"
#include "LightEngine/Engine/Layer.h"
#include "LightEngine/Engine/Patch.h"
#include "LightEngine/Engine/Pools/Stored.h"
#include "LightEngine/Engine/TimeContext.h"
// #include "LightEngine/Show/Sequence.h"      // TODO: playback / cues

// Command subsystem is owned by Engine but kept out of this header (internal):
// forward-declared here, fully included only in Engine.cpp.
namespace LightEngine::Commands
{
class CommandParser;
class CommandExecutor;
} // namespace LightEngine::Commands

//
// Engine: the top-level orchestrator. Owns the Patch (universes + fixtures),
// the programmer, the stored object pools and the output stage, exposing a
// small facade: patch -> select -> program -> store/recall -> update.
// Non-tracking: update() rebuilds every universe from scratch each frame.
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

    // Command subsystem (text -> AST -> actions). Held by pointer so the
    // parser/executor headers stay out of the public API.
    std::unique_ptr<Commands::CommandParser> m_parser;
    std::unique_ptr<Commands::CommandExecutor> m_exec;

public:
    Engine();
    ~Engine(); // out-of-line: m_parser/m_exec are incomplete types here

    // ---- patching (template overload; name overload needs FixtureLibrary)
    // ----
    std::vector<uint16_t>
    patch(const Fixtures::Fixture &fixture, uint16_t universe, uint16_t amount,
          std::optional<uint32_t> start = std::nullopt,
          std::optional<uint16_t> startFID = std::nullopt);

    // ---- stored pools ----
    [[nodiscard]] Stored &stored() { return m_stored; }
    [[nodiscard]] const Stored &stored() const { return m_stored; }

    // ---- text commands ----
    // Load the grammar + token definitions (enables command()).
    void loadCommands(const std::string &tokensFile,
                      const std::string &grammarFile);
    // Parse and execute one command line. Returns false on a parse error (or
    // if loadCommands() hasn't been called).
    bool command(const std::string &line);

    // ---- programmer ----
    // Console-style Clear: wipe the programmer's values and selection so it
    // stops contributing to the frame.
    void clear();

    // Select a stored group into the programmer (replace).
    void selectGroup(uint32_t num);
    // Snapshot the current selection into the group pool at `num` / next free.
    Pools::Group &storeGroup(uint32_t num);
    Pools::Group &storeGroup();

    // ---- color presets ----
    // Capture the current programmer color (h,s) per selected fixture; recall
    // applies it back onto the current selection (stored fixtures only).
    Pools::ColorPreset &storeColorPreset(uint32_t num);
    void recallColorPreset(uint32_t num);

    // ---- dimmer presets ----
    // Capture the current programmer intensity (HSV.v) per selected fixture.
    Pools::DimmerPreset &storeDimmerPreset(uint32_t num);
    void recallDimmerPreset(uint32_t num);

    // ---- lookup ----
    [[nodiscard]] std::shared_ptr<Fixtures::Fixture> getFixture(uint16_t fid);
    [[nodiscard]] DMX::Universe *getUniverse(uint16_t universe);
    [[nodiscard]] Patch &patcher() { return m_patch; }

    // ---- render ----
    // Non-tracking frame: wipe every universe, then push each fixture's state
    // back into its buffer via Resolve() (color cells + generic attributes).
    // dt is the wall-clock delta since the last frame (seconds); the caller
    // owns the clock so the render path stays deterministic and testable.
    void update(float dt = 0.f);

    // ---- output (sACN) ----
    // Configuring a source IP enables the transmit stage; until then update()
    // renders but sends nothing (so headless/print-only runs stay offline).
    void setIP(const std::string &ip);
    void setIP(const Utils::Network::IP &ip);
    void setSourceName(const std::string &name);
    [[nodiscard]] DMXOutput &output() { return m_output; }

    // ---- composed output (read-only, for UIs) ----
    // The merged per-frame values after all layers compose - i.e. what is
    // actually on stage. Valid after update(). Prefer this over programmer
    // edits() for display, since it reflects every layer, not just the user's.
    [[nodiscard]] const Frame &frame() const { return m_frame; }
    [[nodiscard]] const FixtureValues *values(uint16_t fid) const
    {
        return m_frame.get(fid);
    }

    // ---- layers ----
    [[nodiscard]] ProgrammerLayer &programmer() { return m_programmer; }
    void addLayer(Layer *layer) { m_layers.push_back(layer); }

    [[nodiscard]] const TimeContext &time() const { return m_time; }

    [[nodiscard]] std::string describe() const;
};
} // namespace LightEngine::Engine
