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
    mutable std::vector<uint16_t> m_fids;
    mutable bool m_cacheDirty = true;

    // Bumped on every membership change, so a consumer that caches something
    // derived from this selection can detect staleness in O(1) - no re-hashing
    // and no comparing fid lists.
    uint64_t m_revision = 0;

    void rebuildCache() const;

public:
    FixtureGroup() = default;
    explicit FixtureGroup(std::string name);

    // Copying is fine, but being assigned over is a membership change like any
    // other: the revision must move forward, never be overwritten by the
    // source's, or a consumer's cached revision could match by coincidence.
    FixtureGroup(const FixtureGroup &) = default;
    FixtureGroup(FixtureGroup &&) = default;
    FixtureGroup &operator=(const FixtureGroup &other);
    FixtureGroup &operator=(FixtureGroup &&other);
    ~FixtureGroup() = default;

    FixtureGroup operator&(const FixtureGroup &other) {}

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

    // The member FIDs in selection order (cached - effects walk this every
    // frame, so it must not allocate). Valid until the next membership change.
    [[nodiscard]] const std::vector<uint16_t> &fids() const;

    // Order-independent hash over the member fids: two groups holding the
    // same fixtures hash the same regardless of insertion order. For identity
    // (dedup, keying), not for per-frame staleness checks - use revision().
    [[nodiscard]] std::size_t fidsHash() const;

    // Monotonic counter of membership changes. Never reused within a group's
    // lifetime, so `cached == revision()` is a sound cache-validity test.
    [[nodiscard]] uint64_t revision() const { return m_revision; }

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

    FixtureGroup FixtureGroup::operator&(const FixtureGroup &other) const
    {
        FixtureGroup result;

        std::set<uint16_t> otherFids;
        for (const auto &fixture : other.m_fixtures)
        {
            if (fixture)
                otherFids.insert(fixture->Fid());
        }

        for (const auto &fixture : m_fixtures)
        {
            if (fixture && otherFids.contains(fixture->Fid()))
                result.add(fixture);
        }

        return result;
    }

    FixtureGroup FixtureGroup::operator|(const FixtureGroup &other) const
    {
        FixtureGroup result;

        // Left side first, preserving its order.
        result.add(m_fixtures);

        // add() already ignores duplicates, so this appends only fixtures
        // that aren't already present.
        result.add(other.m_fixtures);

        return result;
    }

    [[nodiscard]] std::string describe() const;
};
} // namespace LightEngine::DMX
