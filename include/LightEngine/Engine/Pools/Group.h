#pragma once
#include "LightEngine/DMX/FixtureGroup.h"
#include "PoolObject.h"

namespace LightEngine::Engine::Pools
{

class Group : public Engine::PoolObject
{
    DMX::FixtureGroup
        m_fg; // the working selection — already does the hard part
public:
    Group() = default;
    Group(DMX::FixtureGroup &group) : m_fg(std::move(group)) {};
    explicit Group(std::vector<std::shared_ptr<Fixtures::Fixture>> fixtures)
    {
        m_fg.add(fixtures);
    }

    // direct access — no resolve, it's always live
    DMX::FixtureGroup &fixtureGroup() { return m_fg; }
    const DMX::FixtureGroup &fixtureGroup() const { return m_fg; }

    void add(const std::vector<std::shared_ptr<Fixtures::Fixture>> &fx)
    {
        m_fg.add(fx);
    }

    // snapshot of FIDs — only needed for serialization
    std::vector<uint16_t> fids() const { return m_fg.fids(); }

    std::string describe() const override
    {
        return "Group " + std::to_string(number()) + " \"" + name() + "\" " +
               m_fg.describe();
    }
};
} // namespace LightEngine::Engine::Pools