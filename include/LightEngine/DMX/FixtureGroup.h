#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "Utils/Colors/HSV.h"
#include "Utils/Colors/RGB.h"

#include "LightEngine/Fixture/Fixture.h"
#include "LightEngine/GDTF/LogicalChannel.h"

//
// FixtureGroup: a named selection of fixtures driven together. It does NOT own
// the fixtures (the universes do) - it holds shared references plus a cache of
// their parameters bucketed by Attribute, and a flattened, ordered list of
// their color cells (the target a gradient walks). It also tracks which
// universes the group spans, for dirty-marking.
//
namespace LightEngine::DMX
{
class FixtureGroup
{
    using FixturePtr = std::shared_ptr<LightEngine::Fixtures::Fixture>;

    std::string m_name;
    std::vector<FixturePtr> m_fixtures;
    std::set<uint16_t> m_usedUniverses;

    // caches, rebuilt lazily from m_fixtures (pointers into the fixtures,
    // stable because the fixtures are held by shared_ptr).
    mutable std::map<GDTF::Attribute, std::vector<Fixtures::Parameter *>>
        m_byAttribute;
    mutable std::vector<Fixtures::ColorCell *> m_colorCells;
    mutable bool m_cacheDirty = true;

    void rebuildCache() const;

public:
    FixtureGroup() = default;
    explicit FixtureGroup(std::string name);

    // Membership (ignores nulls and duplicates).
    void add(const FixturePtr &fixture);
    void add(const std::vector<FixturePtr> &fixtures);
    void add(const FixtureGroup &other);
    void clear();

    [[nodiscard]] const std::string &name() const { return m_name; }
    void setName(std::string name) { m_name = std::move(name); }

    [[nodiscard]] const std::vector<FixturePtr> &fixtures() const
    {
        return m_fixtures;
    }

    [[nodiscard]] std::vector<uint16_t> fids() const
    {
        std::vector<uint16_t> fidsarray;
        // fidsarray.resize(m_fixtures.size());
        for (auto &f : m_fixtures)
        {
            fidsarray.push_back(f->Fid());
        }
        return fidsarray;
    }

    [[nodiscard]] std::size_t size() const { return m_fixtures.size(); }
    [[nodiscard]] bool empty() const { return m_fixtures.empty(); }
    [[nodiscard]] bool contains(const FixturePtr &fixture) const;
    [[nodiscard]] const std::set<uint16_t> &usedUniverses() const
    {
        return m_usedUniverses;
    }

    // Every parameter of an attribute across all members (cached).
    [[nodiscard]] const std::vector<Fixtures::Parameter *> &
    parameters(GDTF::Attribute attr) const;
    [[nodiscard]] bool has(GDTF::Attribute attr) const;

    // Flattened, ordered color cells across all members - the gradient target.
    [[nodiscard]] const std::vector<Fixtures::ColorCell *> &colorCells() const;

    // NOTE: a FixtureGroup is a pure selection now - it holds no values. To
    // drive it, hand it to ProgrammerLayer::select() and set values there.

    FixtureGroup &operator+=(const FixturePtr &fixture);
    FixtureGroup &operator+=(const FixtureGroup &other);

    [[nodiscard]] std::string describe() const;
};
} // namespace LightEngine::DMX
