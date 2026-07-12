#include "LightEngine/Commands/CommandExecutor.h"

#include <algorithm>

namespace LightEngine::Commands
{
namespace
{
void appendUnique(std::vector<uint16_t> &acc, const std::vector<uint16_t> &add)
{
    for (uint16_t f : add)
        if (std::find(acc.begin(), acc.end(), f) == acc.end())
            acc.push_back(f);
}
void removeAll(std::vector<uint16_t> &acc, const std::vector<uint16_t> &rem)
{
    for (uint16_t f : rem)
        acc.erase(std::remove(acc.begin(), acc.end(), f), acc.end());
}
} // namespace

void CommandExecutor::run(Macros::Program &program)
{
    for (auto &cmd : program)
        cmd->accept(*this);
}

CommandExecutor::PresetKind CommandExecutor::presetKind(long long bank)
{
    switch (bank)
    {
    case 1:
        return PresetKind::Dimmer;
    case 2:
        return PresetKind::Color;
    default:
        return PresetKind::Unknown;
    }
}

std::vector<uint16_t> CommandExecutor::expand(Macros::Selector &sel) const
{
    std::vector<uint16_t> out;
    if (auto *f = dynamic_cast<Macros::Fixture *>(&sel))
    {
        out.push_back(static_cast<uint16_t>(f->id));
    }
    else if (auto *r = dynamic_cast<Macros::FixtureRange *>(&sel))
    {
        for (long long i = r->from; i <= r->to; ++i)
            out.push_back(static_cast<uint16_t>(i));
    }
    else if (auto *g = dynamic_cast<Macros::Group *>(&sel))
    {
        if (auto grp = m_engine.stored().groups().get(
                static_cast<uint32_t>(g->id)))
            out = grp->fids();
    }
    // Preset is not a fixture selection -> empty.
    return out;
}

std::vector<uint16_t>
CommandExecutor::resolveSelection(Macros::SelectCmd &c) const
{
    std::vector<uint16_t> acc;
    for (auto &item : c.items)
    {
        const auto fids = expand(*item.sel);
        if (item.op == "-")
            removeAll(acc, fids);
        else // "+" or the first item (empty op)
            appendUnique(acc, fids);
    }
    return acc;
}

void CommandExecutor::applyAt(const Macros::AtValue &at)
{
    if (!at.isPreset)
    {
        // 'at' levels are 0..100; the programmer takes 0..1.
        m_engine.programmer().setIntensity(
            static_cast<float>(at.level / 100.0));
        return;
    }
    auto *p = dynamic_cast<Macros::Preset *>(at.preset.get());
    if (!p)
        return;
    switch (presetKind(p->bank))
    {
    case PresetKind::Color:
        m_engine.recallColorPreset(static_cast<uint32_t>(p->number));
        break;
    case PresetKind::Dimmer:
        m_engine.recallDimmerPreset(static_cast<uint32_t>(p->number));
        break;
    case PresetKind::Unknown:
        break;
    }
}

// ---- CommandVisitor ---------------------------------------------------------
void CommandExecutor::visit(Macros::SelectCmd &c)
{
    m_engine.programmer().select(resolveSelection(c));
    if (c.hasAt)
        applyAt(c.at);
}

void CommandExecutor::visit(Macros::StoreCmd &c)
{
    if (auto *g = dynamic_cast<Macros::Group *>(c.target.get()))
    {
        m_engine.storeGroup(static_cast<uint32_t>(g->id));
    }
    else if (auto *p = dynamic_cast<Macros::Preset *>(c.target.get()))
    {
        switch (presetKind(p->bank))
        {
        case PresetKind::Color:
            m_engine.storeColorPreset(static_cast<uint32_t>(p->number));
            break;
        case PresetKind::Dimmer:
            m_engine.storeDimmerPreset(static_cast<uint32_t>(p->number));
            break;
        case PresetKind::Unknown:
            break;
        }
    }
}

void CommandExecutor::visit(Macros::DeleteCmd &c)
{
    if (auto *g = dynamic_cast<Macros::Group *>(c.target.get()))
    {
        m_engine.stored().groups().remove(static_cast<uint32_t>(g->id));
    }
    else if (auto *p = dynamic_cast<Macros::Preset *>(c.target.get()))
    {
        switch (presetKind(p->bank))
        {
        case PresetKind::Color:
            m_engine.stored().colorPresets().remove(
                static_cast<uint32_t>(p->number));
            break;
        case PresetKind::Dimmer:
            m_engine.stored().dimmerPresets().remove(
                static_cast<uint32_t>(p->number));
            break;
        case PresetKind::Unknown:
            break;
        }
    }
}

void CommandExecutor::visit(Macros::ClearCmd &) { m_engine.clear(); }

void CommandExecutor::visit(Macros::AtCmd &c) { applyAt(c.at); }

} // namespace LightEngine::Commands
