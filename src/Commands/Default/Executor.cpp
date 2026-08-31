#include "LightEngine/Commands/Default/Executor.h"

#include "Utils/Colors/Colors.h"

#include <algorithm>
#include <cstdint>

namespace LightEngine::Commands::Default
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

void CommandExecutor::run(Program &program)
{
    for (auto &cmd : program)
        cmd->accept(*this);
}

std::vector<uint16_t> CommandExecutor::expand(Selector &sel) const
{
    std::vector<uint16_t> out;
    if (auto *f = dynamic_cast<Fixture *>(&sel))
    {
        out.push_back(static_cast<uint16_t>(f->id));
    }
    else if (auto *r = dynamic_cast<FixtureRange *>(&sel))
    {
        for (long long i = r->from; i <= r->to; ++i)
            out.push_back(static_cast<uint16_t>(i));
    }
    else if (auto *g = dynamic_cast<Group *>(&sel))
    {
        if (auto grp = m_engine.presets().groups().get(
                static_cast<uint32_t>(g->id)))
            out = grp->fids();
    }
    // Preset is not a fixture selection -> empty.
    return out;
}

std::vector<uint16_t>
CommandExecutor::resolveSelection(SelectCmd &c) const
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

void CommandExecutor::applyAt(const AtValue &at)
{
    if (at.kind == "level")
    {
        // 'at' levels are 0..100; the programmer takes 0..1.
        m_engine.programmer().setIntensity(
            static_cast<float>(at.level / 100.0));
        return;
    }
    if (at.kind == "color")
    {
        // 'at color r,g,b' is a full RGB value: hue/sat AND brightness (the
        // value channel), so it lights up on its own without a prior 'at <n>'.
        // const Utils::Colors::HSV hsv = Utils::Colors::rgbToHsv(
            auto rgb = Utils::Colors::RGB(static_cast<uint8_t>(at.r),
                               static_cast<uint8_t>(at.g),
                               static_cast<uint8_t>(at.b));
        m_engine.programmer().setColor(rgb);
        return;
    }
    // kind == "preset"
    auto *p = dynamic_cast<Preset *>(at.preset.get());
    if (!p)
        return;
    if (p->kind == "color")
        m_engine.recallColorPreset(static_cast<uint32_t>(p->number));
    else
        m_engine.recallDimmerPreset(static_cast<uint32_t>(p->number));
}

// ---- CommandVisitor ---------------------------------------------------------
void CommandExecutor::visit(SelectCmd &c)
{
    m_engine.programmer().select(resolveSelection(c));
    if (c.hasAt)
        applyAt(c.at);
}

void CommandExecutor::visit(StoreCmd &c)
{
    if (auto *g = dynamic_cast<Group *>(c.target.get()))
    {
        m_engine.storeGroup(static_cast<uint32_t>(g->id));
    }
    else if (auto *p = dynamic_cast<Preset *>(c.target.get()))
    {
        if (p->kind == "color")
            m_engine.storeColorPreset(static_cast<uint32_t>(p->number));
        else
            m_engine.storeDimmerPreset(static_cast<uint32_t>(p->number));
    }
}

void CommandExecutor::visit(DeleteCmd &c)
{
    if (auto *g = dynamic_cast<Group *>(c.target.get()))
    {
        m_engine.presets().groups().remove(static_cast<uint32_t>(g->id));
    }
    else if (auto *p = dynamic_cast<Preset *>(c.target.get()))
    {
        if (p->kind == "color")
            m_engine.presets().presets(Effects::EffectCategory::COLOR).remove(
                static_cast<uint32_t>(p->number));
        else
            m_engine.presets().presets(Effects::EffectCategory::DIMMER).remove(
                static_cast<uint32_t>(p->number));
    }
}

void CommandExecutor::visit(ClearCmd &) { m_engine.clear(); }

void CommandExecutor::visit(AtCmd &c) { applyAt(c.at); }

} // namespace LightEngine::Commands