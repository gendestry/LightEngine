#include "LightEngine/Engine/Components/Patch.h"

using namespace LightEngine::Engine::Components;

LightEngine::DMX::UniversePatch &Patch::getUniverse(uint16_t universe)
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
        m_fixtureByUID[m_fixCurrentID] = std::move(fix);
        fixtures.push_back(m_fixtureByUID[m_fixCurrentID++]);
    }

    return fixtures;
}

std::vector<uint16_t> Patch::patch(Fixtures::Fixture *fixtemplate, uint16_t universe,
                                   uint16_t amount, std::optional<uint32_t> start,
                                   std::optional<uint16_t> startFID)
{
    if (amount == 0)
    {
        logger.error("Trying to patch 0 fixtures");
        return {};
    }

    const auto &fixture = *fixtemplate;
    logger.debug("here");

    LightEngine::DMX::UniversePatch &uni = getUniverse(universe);

    uint32_t addr = start.has_value() ? *start : 0;
    auto placed = uni.addMultiple(fixture.size, addr, amount);

    if (placed.empty())
    {
        logger.error("Something went wrong");
        return {};
    }

    auto previd = m_fixCurrentID;
    auto fixtures = addFixtures(fixture, amount);

    std::vector<uint16_t> fids;
    fids.reserve(amount);

    for (int i = 0; i < amount; i++)
    {
        auto f = fixtures[i];
        auto &pinfo = placed[i].get();

        f->setBuffer(uni.getRaw());
        f->setStart(pinfo.start);
        f->SetFid(previd++);
        f->SetUniverse(universe);
        fids.push_back(f->Fid());
    }

    return fids;

    // std::vector<FixturePtr> placed;

    // if (start.has_value())
    // {
    //     placed = uni.addFixtures(fixture, amount, *start);
    // }
    // else
    // {
    //     placed.reserve(amount);
    //     for (uint16_t i = 0; i < amount; ++i)
    //     {
    //         placed.push_back(uni.addFixture(fixture));
    //     }
    // }

    // for (auto fix : placed)
    // {
    //     fix->SetFid(m_fixCurrentID);
    //     m_fixtureByUID[m_fixCurrentID++] = std::move(fix);
    // }
    // return {};

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
