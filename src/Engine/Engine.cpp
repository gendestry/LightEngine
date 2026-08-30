#include "LightEngine/Engine/Engine.h"

#include "LightEngine/Effects/Static/StaticSnapshot.h"

// #include "LightEngine/Commands/CommandExecutor.h"
// #include "LightEngine/Commands/CommandParser.h"

namespace LightEngine::Engine
{
Engine::Engine() : m_logger("Engine")
{
    m_output.setIP(m_config.ip);
    m_output.setSourceName(m_config.name);
    m_logger.setLoggerLevel(Utils::Logger::DEBUGGING);
}
Engine::~Engine() = default; // here the command types are complete

// ---- text commands ----
// void Engine::loadCommands(const std::string &tokensFile,
//                           const std::string &grammarFile)
// {
//     m_parser = std::make_unique<Commands::CommandParser>(tokensFile,
//     grammarFile); m_exec =
//     std::make_unique<Commands::CommandExecutor>(*this);
// }

// bool Engine::command(const std::string &line)
// {
//     if (!m_parser || !m_exec)
//         return false;
//     auto program = m_parser->parse(line);
//     if (program.empty())
//         return false;
//     m_exec->run(program);
//     return true;
// }

// ---- patching ----
// std::vector<uint16_t>
// Engine::patch(std::shared_ptr<Fixtures::FixtureTemplate> fixtureTemp, uint16_t universe,
//               uint16_t amount, std::optional<uint32_t> start,
//               std::optional<uint16_t> startFID)
// {
//     return m_patch.patch(Fixtures::Fixture(fixtureTemp), universe, amount, start, startFID);
// }

// ---- programmer ----
// void Engine::clear() { m_programmer.clearAll(); }

// void Engine::selectGroup(uint32_t num)
// {
//     if (auto grp = m_presets.groups().get(num))
//     {
//         m_programmer.select(grp->fixtureGroup());
//         m_logger.debug("Selected Group {} {}", num, grp->fixtureGroup().toString());
//     }
//     else
//     {
//         m_logger.warn("Group {} does not exist", num);
//     }
// }

// void Engine::appendGroup(uint32_t num)
// {
//     if (auto grp = m_presets.groups().get(num))
//     {
//         m_programmer.add(grp->fixtureGroup());
//         m_logger.debug("Appended Group {} {}", num, m_programmer.selected().toString());
//     }
//     else
//     {
//         m_logger.warn("Group {} does not exist", num);
//     }
// }

Pools::Group &Engine::storeGroup(uint32_t num)
{
    return m_presets.groups().emplaceAt(num, m_programmer.selected());
}

Pools::Group &Engine::storeGroup()
{
    return m_presets.groups().emplace(m_programmer.selected());
}

// ---- presets ----
// Store: snapshot the current selection's static picture for one attribute
// (see Programmer::snapshotStatic / EffectGroup::snapshotStatic for what
// "static" excludes) and wrap it in a fresh StaticSnapshot.
namespace
{
std::shared_ptr<Effects::StaticSnapshot>
makeSnapshot(Components::Programmer &programmer, Effects::EffectCategory cat,
            const Utils::Time::TimeContext &time)
{
    return std::make_shared<Effects::StaticSnapshot>(
        programmer.snapshotStatic(cat, time), cat);
}
} // namespace

Pools::Preset &Engine::storeColorPreset(uint32_t num)
{
    return m_presets.presets(Effects::EffectCategory::COLOR)
        .emplaceAt(num, makeSnapshot(m_programmer, Effects::EffectCategory::COLOR, m_time));
}

Pools::Preset &Engine::storeColorPreset()
{
    return m_presets.presets(Effects::EffectCategory::COLOR)
        .emplace(makeSnapshot(m_programmer, Effects::EffectCategory::COLOR, m_time));
}

Pools::Preset &Engine::storeDimmerPreset(uint32_t num)
{
    return m_presets.presets(Effects::EffectCategory::DIMMER)
        .emplaceAt(num, makeSnapshot(m_programmer, Effects::EffectCategory::DIMMER, m_time));
}

Pools::Preset &Engine::storeDimmerPreset()
{
    return m_presets.presets(Effects::EffectCategory::DIMMER)
        .emplace(makeSnapshot(m_programmer, Effects::EffectCategory::DIMMER, m_time));
}

// Recall: mask the preset's own value table by whatever is selected *now* -
// recalling on a subset only touches that subset, because StaticSnapshot's
// recompute() intersects its table against the group it's evaluated with.
namespace
{
void recallPreset(Components::Programmer &programmer, Effects::EffectCategory cat,
                  const Components::Presets &presets, uint32_t num,
                  Utils::Logger &logger)
{
    auto preset = presets.presets(cat).get(num);
    if (!preset)
    {
        logger.warn("Preset {} does not exist", num);
        return;
    }
    programmer.applyEffect(std::make_shared<Effects::EffectWrapper>(
        Effects::EffectWrapper{programmer.selected(), preset->effect()->clone()}));
}
} // namespace

void Engine::recallColorPreset(uint32_t num)
{
    recallPreset(m_programmer, Effects::EffectCategory::COLOR, m_presets, num, m_logger);
}

void Engine::recallDimmerPreset(uint32_t num)
{
    recallPreset(m_programmer, Effects::EffectCategory::DIMMER, m_presets, num, m_logger);
}

bool Engine::renameGroup(uint32_t num, const std::string &name)
{
    return m_presets.groups().rename(num, name);
}

bool Engine::renamePreset(Effects::EffectCategory cat, uint32_t num, const std::string &name)
{
    return m_presets.presets(cat).rename(num, name);
}

// // ---- lookup ----
// std::shared_ptr<Fixtures::Fixture> Engine::getFixture(uint16_t fid)
// {
//     return m_patch.getFixture(fid);
// }

// DMX::Universe *Engine::getUniverse(uint16_t universe)
// {
//     return m_patch.getUniverse(universe);
// }

// // ---- resolved DMX read-back ----
// uint16_t Engine::attributeValue(uint16_t fid, GDTF::Attribute attr,
//                                 uint16_t cell)
// {
//     auto fixture = m_patch.getFixture(fid);
//     if (!fixture)
//         return 0;

//     const auto &byAttr = fixture->ByAttribute();
//     auto it = byAttr.find(attr);
//     if (it == byAttr.end())
//         return 0;

//     const DMX::Universe *uni = m_patch.getUniverse(fixture->Universe());
//     if (!uni)
//         return 0;

//     for (const Fixtures::Parameter *p : it->second)
//     {
//         if (p->CellIndex() != cell)
//             continue;

//         const auto &buf = uni->buffer();
//         const auto &ch = p->Definition()->channel;
//         uint32_t addr = fixture->start + ch.address;
//         if (ch.res == GDTF::DMXChannel::Resolution::Bit8)
//             return buf[addr];
//         return (uint16_t(buf[addr]) << 8) | buf[addr + 1]; // 16-bit,
//         big-endian
//     }
//     return 0;
// }

// Utils::Colors::RGB Engine::color(uint16_t fid, uint16_t cell)
// {
//     return {static_cast<uint8_t>(attributeValue(fid,
//     GDTF::Attribute::COLOR_R, cell)),
//             static_cast<uint8_t>(attributeValue(fid,
//             GDTF::Attribute::COLOR_G, cell)),
//             static_cast<uint8_t>(attributeValue(fid,
//             GDTF::Attribute::COLOR_B, cell))};
// }

// // ---- output ----
// void Engine::setIP(const std::string &ip)
// {
//     m_output.setIP(ip);
//     m_outputEnabled = true;
// }

// void Engine::setIP(const Utils::Network::IP &ip)
// {
//     m_output.setIP(ip);
//     m_outputEnabled = true;
// }

// void Engine::setSourceName(const std::string &name)
// {
//     m_output.setSourceName(name);
// }

void Engine::update(float dt)
{
    // advance the frame clock before composing, so every layer samples a
    // consistent "now" for the whole frame.
    m_time.dt = dt;
    m_time.now += dt;
    ++m_time.frame;

    m_patch.clearDMXBuffers();

    // 1. compose: layers write their contributions into the frame, composed
    // in
    //    priority order (low -> high) so higher layers' LTP writes win.
    m_frame.clear();
    // std::stable_sort(m_layers.begin(), m_layers.end(),
    //                  [](const Layer *a, const Layer *b)
    //                  { return a->priority() < b->priority(); });
    // for (Layer *layer : m_layers)
    // {
    //     if (layer->enabled())
    //     {
    //         layer->apply(m_frame, m_time);
    //     }
    // }
    m_programmer.apply(m_frame, m_time);

    // 2. resolve: push each fixture's merged values into its DMX buffer.
    //    Fixtures are stateless sinks - anything not addressed this frame stays
    //    at the blackout value.
    // auto &fixs = m_patch.fixs();
    for (const auto &[fid, values] : m_frame.all())
    {
        for (const auto &fixture : m_patch.fixturesByFID(fid))
        {
            fixture->Resolve(values);
        }
    }
    // for (const auto &[fid, values] : m_frame.all())
    // {
    //     // if (auto fixture = m_patch.getFixture(fid))
    //     // {
    //     fixs[fid]->Resolve(values);
    //     // }
    // }

    // 3. output: continuous full-frame send, as a real sACN source does.
    // Only
    //    once an IP has been configured (setIP), else this is a render-only
    //     run.
    if (m_config.output)
    {
        m_output.sendAll(m_patch);
    }
    // m_patch.clearDirty();
}

std::string Engine::toString() const
{
    Utils::Text::Stream s;
    s << m_config.toString() << "\n";
    s << m_patch.toString();
    return s.end();
}
} // namespace LightEngine::Engine
