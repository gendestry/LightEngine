#include "LightEngine/Engine/Components/Patch.h"

using namespace LightEngine::Engine::Components;

LightEngine::DMX::Universe &Patch::getUniverse(uint16_t universe)
{
    auto it = m_universes.find(universe);
    if (it == m_universes.end())
    {
        it = m_universes.try_emplace(universe, universe).first;
    }
    return it->second;
}

std::vector<Patch::FixturePtr> Patch::addFixtures(const Fixtures::Fixture &fixture, uint32_t amount)
{
    std::vector<FixturePtr> fixtures;
    fixtures.reserve(amount);
    for (int i = 0; i < amount; i++)
    {
        auto fix = std::make_shared<Fixtures::Fixture>(fixture);
        m_fixtureByUID[fix->id] = std::move(fix);
        fixtures.push_back(m_fixtureByUID[fix->id]);
    }

    return fixtures;
}

std::vector<uint16_t> Patch::patch(const Fixtures::Fixture &fixture, uint16_t universe,
                                   uint16_t amount, std::optional<uint32_t> start,
                                   std::optional<uint16_t> startFID)
{
    if (amount == 0)
    {
        logger.error("Trying to patch 0 fixtures");
        return {};
    }

    uint32_t addr = start.has_value() ? *start : 0;
    logger.debug("Here");

    LightEngine::DMX::Universe &uni = getUniverse(universe);
    logger.debug("{}", fixture.toString());

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

    for (auto fix : placed)
    {
        fix->SetFid(fix->getUID());
        m_fixtureByUID[fix->getUID()] = std::move(fix);
    }
    return {};

    // TODO: temp
}

std::string Patch::toString() const
{
    std::string ret = "Patch\n";
    for (auto &[k, v] : m_universes)
    {
        ret += v.dump();
    }

    return ret;
}
