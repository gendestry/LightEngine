#include "LightEngine/Engine/Patch.h"

#include <utility>

namespace LightEngine::Engine
{
LightEngine::DMX::Universe &Patch::ensureUniverse(uint16_t universe)
{
    auto it = m_universes.find(universe);
    if (it == m_universes.end())
    {
        it = m_universes.try_emplace(universe, universe).first;
    }
    return it->second;
}

uint16_t Patch::nextFreeFid() const
{
    uint16_t fid = 1;
    while (m_usedFids.contains(fid))
    {
        ++fid;
    }
    return fid;
}

void Patch::registerFixture(uint16_t fid, const FixturePtr &fixture)
{
    fixture->SetFid(fid);
    // NOTE: no ensureVirtualDimmer() - the virtual dimmer is folded into
    // ColorCell::Resolve() in this engine, so color-only fixtures need nothing.
    m_fixtures[fid] = fixture;
    m_usedFids.insert(fid);
    m_byName[fixture->Name()].push_back(fixture);
}

std::vector<uint16_t>
Patch::patch(const LightEngine::Fixtures::Fixture &fixture, uint16_t universe,
             uint16_t amount, std::optional<uint32_t> start,
             std::optional<uint16_t> startFID)
{
    if (amount == 0)
    {
        return {};
    }

    LightEngine::DMX::Universe &uni = ensureUniverse(universe);

    std::vector<FixturePtr> placed;
    if (start.has_value())
    {
        placed = uni.addFixtures(fixture, amount, *start);
    }
    else
    {
        placed.reserve(amount);
        for (uint16_t i = 0; i < amount; ++i)
        {
            placed.push_back(uni.addFixture(fixture));
        }
    }

    std::vector<uint16_t> fids;
    fids.reserve(placed.size());
    uint16_t fid = startFID.value_or(nextFreeFid());
    for (const auto &f : placed)
    {
        if (f == nullptr)
        {
            continue; // placement failed (overlap / full)
        }
        while (m_usedFids.contains(fid))
        {
            ++fid;
        }
        registerFixture(fid, f);
        fids.push_back(fid);
        ++fid;
    }

    if (!fids.empty())
    {
        markDirty(universe);
    }
    return fids;
}

LightEngine::DMX::Universe *Patch::getUniverse(uint16_t universe)
{
    const auto it = m_universes.find(universe);
    return it != m_universes.end() ? &it->second : nullptr;
}

Patch::FixturePtr Patch::getFixture(uint16_t fid) const
{
    const auto it = m_fixtures.find(fid);
    return it != m_fixtures.end() ? it->second : nullptr;
}

std::vector<Patch::FixturePtr>
Patch::getFixtures(const std::vector<uint16_t> &fids) const
{
    std::vector<FixturePtr> out;
    out.reserve(fids.size());
    for (uint16_t fid : fids)
    {
        if (auto f = getFixture(fid))
        {
            out.push_back(std::move(f));
        }
    }
    return out;
}

const std::vector<Patch::FixturePtr> &
Patch::getFixturesByName(const std::string &name) const
{
    static const std::vector<FixturePtr> empty;
    const auto it = m_byName.find(name);
    return it != m_byName.end() ? it->second : empty;
}

std::string Patch::describe() const
{
    std::string s = "Patch [" + std::to_string(m_fixtures.size()) +
                    " fixtures across " + std::to_string(m_universes.size()) +
                    " universes]\n";
    for (const auto &[id, uni] : m_universes)
    {
        s += uni.describe();
    }
    return s;
}
} // namespace LightEngine::Engine
