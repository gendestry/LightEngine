#pragma once
#include "LightEngine/DMX/FixtureGroup.h"
#include "PoolObject.h"

namespace LightEngine::Engine::Pools
{

class Group : public PoolObject
{
    // DMX::FixtureGroup
    //     m_fg; // the working selection — already does the hard part
    Utils::Maths::Interval m_group;

public:
    Group() = default;
    Group(const Utils::Maths::Interval &group) : m_group(std::move(group)) {};
    // explicit Group(std::vector<std::shared_ptr<Fixtures::Fixture>> fixtures)
    // {
    //     m_fg.add(fixtures);
    // }

    // direct access — no resolve, it's always live
    // DMX::FixtureGroup &fixtureGroup() { return m_fg; }
    // const DMX::FixtureGroup &fixtureGroup() const { return m_fg; }

    Utils::Maths::Interval &fixtureGroup() { return m_group; }
    const Utils::Maths::Interval &fixtureGroup() const { return m_group; }

    // void add(const std::vector<std::shared_ptr<Fixtures::Fixture>> &fx)
    // {
    //     m_fg.add(fx);
    // }

    // snapshot of FIDs — only needed for serialization
    std::vector<uint16_t> fids() const
    {
        const auto &values = m_group.values();
        std::vector<uint16_t> ret;
        ret.reserve(values.size());
        for (const auto v : values)
            ret.push_back(static_cast<uint16_t>(v));
        return ret;
    }

    [[nodiscard]] std::string toString() const override
    {
        // nameExpr is pre-formatted to a plain string, not spliced into the
        // outer format string as a literal - that keeps exactly one "{}" per
        // value below regardless of whether the object is named, so the
        // fixture list can't silently absorb the empty name() argument (the
        // bug this replaced: an unnamed object's "Unnamed" literal had no
        // placeholder, which shifted every later {} one argument to the left).
        const std::string nameExpr =
            name().empty() ? Theme::dim("Unnamed") : Utils::Font::format(Theme::txt("{}"), name());

        return Utils::Font::format(
            Utils::Font::group(Theme::lbl("Group "), "[", Theme::num("{}"), "]: ", "{}", " fixs: {}"),
            number(), nameExpr, m_group.toString());
    }
};
} // namespace LightEngine::Engine::Pools