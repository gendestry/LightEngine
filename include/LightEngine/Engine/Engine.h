#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "LightEngine/Effects/Engine.h"
#include "LightEngine/Engine/EngineConfig.h"
#include "LightEngine/Engine/Layers/Frame.h"
#include "LightEngine/Engine/Layers/Layer.h"
#include "LightEngine/Engine/Layers/Programmer.h"
#include "LightEngine/Engine/Patch.h"
#include "LightEngine/Engine/Pools/Presets.h"
#include "LightEngine/Engine/TimeContext.h"
#include "LightEngine/FixtureTools/FixtureLibrary.h"
#include "LightEngine/Output/DMXOutput.h"
#include "Utils/Logging/Logger.h"
// #include "Utils/Colors/RGB.h"
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
class Engine : public Utils::Traits::Stringify
{
    Config m_config;
    Patch m_patch;
    TimeContext m_time; // advanced each update(); threaded into layers
    FixtureLibrary m_library;

    Output::DMXOutput m_output; // sACN transmit stage

    Frame m_frame;                      // per-frame merged values (rebuilt each tick)
    Programmer m_programmer;            // live editing layer
    std::vector<Layer *> m_layers = {}; // composed low -> high

    Presets m_presets; // all object pools (groups, presets, cues...) live here
    Utils::Logger logger;

public:
    Engine();
    ~Engine(); // out-of-line: m_parser/m_exec are incomplete types here

    std::vector<uint16_t>
    patch(std::shared_ptr<Fixtures::Fixture> fix, uint16_t universe, uint16_t amount,
          std::optional<uint32_t> start = std::nullopt,
          std::optional<uint16_t> startFID = std::nullopt);

    // // Select a stored group into the programmer (replace).
    void selectGroup(uint32_t num);
    void appendGroup(uint32_t num);
    // // Snapshot the current selection into the group pool at `num` / next
    Pools::Group &storeGroup(uint32_t num);
    Pools::Group &storeGroup();

    Pools::Color &storeColorPreset(uint32_t num);
    Pools::Color &storeColorPreset();
    void recallColorPreset(uint32_t num);

    void update(float dt = 0.f);

    // // ---- layers ----
    [[nodiscard]] Programmer &programmer() { return m_programmer; }
    void addLayer(Layer *layer) { m_layers.push_back(layer); }

    [[nodiscard]] const TimeContext &time() const { return m_time; }
    [[nodiscard]] const Patch &patcher() const { return m_patch; }
    [[nodiscard]] const Presets &presets() const { return m_presets; }
    [[nodiscard]] FixtureLibrary &fixtureLibrary()
    {
        return m_library;
    }

    [[nodiscard]] std::string toString() const override;
};
} // namespace LightEngine::Engine
