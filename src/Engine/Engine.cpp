#include "LightEngine/Engine/Engine.h"

#include "LightEngine/Commands/CommandExecutor.h"
#include "LightEngine/Commands/CommandParser.h"

#include <algorithm>

namespace LightEngine::Engine
{
Engine::Engine() : m_programmer(m_patch) {} // m_patch declared first -> safe
Engine::~Engine() = default; // here the command types are complete

// ---- text commands ----
void Engine::loadCommands(const std::string &tokensFile,
                          const std::string &grammarFile)
{
    m_parser = std::make_unique<Commands::CommandParser>(tokensFile, grammarFile);
    m_exec = std::make_unique<Commands::CommandExecutor>(*this);
}

bool Engine::command(const std::string &line)
{
    if (!m_parser || !m_exec)
        return false;
    auto program = m_parser->parse(line);
    if (program.empty())
        return false;
    m_exec->run(program);
    return true;
}

// ---- patching ----
std::vector<uint16_t> Engine::patch(const Fixtures::Fixture &fixture,
                                    uint16_t universe, uint16_t amount,
                                    std::optional<uint32_t> start,
                                    std::optional<uint16_t> startFID)
{
    return m_patch.patch(fixture, universe, amount, start, startFID);
}

// ---- programmer ----
void Engine::clear() { m_programmer.clearAll(); }

void Engine::selectGroup(uint32_t num)
{
    if (auto grp = m_stored.groups().get(num))
        m_programmer.select(grp->fixtureGroup());
}

Pools::Group &Engine::storeGroup(uint32_t num)
{
    return m_stored.groups().emplaceAt(num,
                                       m_programmer.selection().fixtures());
}

Pools::Group &Engine::storeGroup()
{
    return m_stored.groups().emplace(m_programmer.selection().fixtures());
}

// ---- color presets ----
// Captures the whole programmer, not just the current selection: every fixture
// touched since the last clear() is banked, so a preset built across several
// selections (group then a stray fixture) keeps them all.
Pools::ColorPreset &Engine::storeColorPreset(uint32_t num)
{
    auto preset = std::make_shared<Pools::ColorPreset>();
    for (const auto &[fid, v] : m_programmer.edits())
        if (v.color)
            preset->set(fid, {v.color->h, v.color->s});
    return m_stored.colorPresets().store(num, std::move(preset));
}

void Engine::recallColorPreset(uint32_t num)
{
    if (auto preset = m_stored.colorPresets().get(num))
        preset->recall(m_programmer, m_programmer.selection());
}

// ---- dimmer presets ----
Pools::DimmerPreset &Engine::storeDimmerPreset(uint32_t num)
{
    auto preset = std::make_shared<Pools::DimmerPreset>();
    for (const auto &[fid, v] : m_programmer.edits())
        if (v.intensity)
            preset->set(fid, *v.intensity);
    return m_stored.dimmerPresets().store(num, std::move(preset));
}

void Engine::recallDimmerPreset(uint32_t num)
{
    if (auto preset = m_stored.dimmerPresets().get(num))
        preset->recall(m_programmer, m_programmer.selection());
}

// ---- lookup ----
std::shared_ptr<Fixtures::Fixture> Engine::getFixture(uint16_t fid)
{
    return m_patch.getFixture(fid);
}

DMX::Universe *Engine::getUniverse(uint16_t universe)
{
    return m_patch.getUniverse(universe);
}

// ---- output ----
void Engine::setIP(const std::string &ip)
{
    m_output.setIP(ip);
    m_outputEnabled = true;
}

void Engine::setIP(const Utils::Network::IP &ip)
{
    m_output.setIP(ip);
    m_outputEnabled = true;
}

void Engine::setSourceName(const std::string &name)
{
    m_output.setSourceName(name);
}

std::string Engine::describe() const { return m_patch.describe(); }

void Engine::update(float dt)
{
    // advance the frame clock before composing, so every layer samples a
    // consistent "now" for the whole frame.
    m_time.dt = dt;
    m_time.now += dt;
    ++m_time.frame;

    m_patch.blackout();

    // 1. compose: layers write their contributions into the frame, composed in
    //    priority order (low -> high) so higher layers' LTP writes win.
    m_frame.clear();
    std::stable_sort(m_layers.begin(), m_layers.end(),
                     [](const Layer *a, const Layer *b)
                     { return a->priority() < b->priority(); });
    for (Layer *layer : m_layers)
    {
        if (layer->enabled())
        {
            layer->apply(m_frame, m_time);
        }
    }

    // 2. resolve: push each fixture's merged values into its DMX buffer.
    //    Fixtures are stateless sinks - anything not addressed this frame stays
    //    at the blackout value.
    for (const auto &[fid, values] : m_frame.all())
    {
        if (auto fixture = m_patch.getFixture(fid))
        {
            fixture->Resolve(values);
        }
    }

    // 3. output: continuous full-frame send, as a real sACN source does. Only
    //    once an IP has been configured (setIP), else this is a render-only run.
    if (m_outputEnabled)
    {
        m_output.sendAll(m_patch);
    }
    m_patch.clearDirty();
}
} // namespace LightEngine::Engine
