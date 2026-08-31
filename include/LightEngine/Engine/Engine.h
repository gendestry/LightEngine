#pragma once

#include <string>
#include <vector>

#include "LightEngine/Engine/Components/FixtureLibrary.h"
#include "LightEngine/Engine/Components/Patch.h"
#include "LightEngine/Engine/Components/Presets.h"
#include "LightEngine/Engine/Components/Programmer.h"
#include "LightEngine/Engine/EngineConfig.h"
#include "LightEngine/Engine/Layers/Frame.h"
#include "LightEngine/Engine/Layers/Layer.h"
#include "LightEngine/Output/DMXOutput.h"
#include "Utils/Logging/Logger.h"
#include "Utils/Time/TimeContext.h"
#include <memory>

// Command subsystem is owned by Engine but kept out of this header (internal):
// forward-declared here, fully included only in Engine.cpp.
namespace LightEngine::Commands::Default
{
class CommandParser;
class CommandExecutor;
} // namespace LightEngine::Commands::Default

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

    Components::FixtureLibrary m_library;
    Components::Patch m_patch;
    Components::Programmer m_programmer; // live editing layer
    Components::Presets m_presets;       // all object pools (groups, presets, cues...) live here

    Output::DMXOutput m_output; // sACN transmit stage

    Frame m_frame;                      // per-frame merged values (rebuilt each tick)
    std::vector<Layer *> m_layers = {}; // composed low -> high

    Utils::Logger m_logger;
    Utils::Time::TimeContext m_time; // advanced each update(); threaded into layers

    std::unique_ptr<Commands::Default::CommandParser> m_parser;
    std::unique_ptr<Commands::Default::CommandExecutor> m_exec;

public:
    Engine();
    ~Engine(); // out-of-line: m_parser/m_exec are incomplete types here

    // Load the grammar + token definitions (enables command()).
    void loadCommands(const std::string &tokensFile,
                      const std::string &grammarFile);
    // Parse and execute one command line. Returns false on a parse error (or
    // if loadCommands() hasn't been called).
    bool command(const std::string &line);

    // // Select a stored group into the programmer (replace).
    // void selectGroup(uint32_t num);
    // void appendGroup(uint32_t num);
    // // Snapshot the current selection into the group pool at `num` / next
    Pools::Group &storeGroup(uint32_t num);
    Pools::Group &storeGroup();

    // Snapshot the current selection's static picture for one attribute -
    // fixtures a running animated effect in that category owns are excluded.
    // See EffectGroup::snapshotStatic.
    Pools::Preset &storeColorPreset(uint32_t num);
    Pools::Preset &storeColorPreset();
    Pools::Preset &storeDimmerPreset(uint32_t num);
    Pools::Preset &storeDimmerPreset();

    void recallColorPreset(uint32_t num);
    void recallDimmerPreset(uint32_t num);

    // Rename a stored object. false if `num` doesn't exist in that pool.
    // Presets are numbered per-category (color 1 and dimmer 1 are distinct
    // objects), so a preset rename has to say which pool it means.
    bool renameGroup(uint32_t num, const std::string &name);
    bool renamePreset(Effects::EffectCategory cat, uint32_t num, const std::string &name);

    void update(float dt = 0.f);

    // Deselect and drop any running effects in the programmer (console-style
    // "Clear" button) - does not touch stored groups/presets.
    void clear();

    // // ---- layers ----
    void addLayer(Layer *layer) { m_layers.push_back(layer); }

    [[nodiscard]] const Utils::Time::TimeContext &time() const { return m_time; }
    [[nodiscard]] Components::Patch &patcher() { return m_patch; }
    [[nodiscard]] Components::Programmer &programmer() { return m_programmer; }
    [[nodiscard]] Components::Presets &presets() { return m_presets; }
    [[nodiscard]] Components::FixtureLibrary &fixtureLibrary()
    {
        return m_library;
    }

    [[nodiscard]] std::string toString() const override;
};
} // namespace LightEngine::Engine
