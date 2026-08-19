#include "LightEngine/DMX/FixtureGroup.h"

#include <algorithm>
#include <utility>

#include "Utils/Colors/Colors.h"

namespace LightEngine::DMX
{
FixtureGroup::FixtureGroup(std::string name) : m_name(std::move(name)) {}

FixtureGroup &FixtureGroup::operator=(const std::vector<FixturePtr> &fixs)
{
    add(fixs);
    return *this;
}

FixtureGroup &FixtureGroup::operator=(const FixtureGroup &other)
{
    if (this != &other)
    {
        const uint64_t rev = m_revision;
        m_name = other.m_name;
        m_fixtures = other.m_fixtures;
        m_usedUniverses = other.m_usedUniverses;
        m_cacheDirty =
            true; // the cached pointers/fids describe the old members
        m_revision = rev + 1;
    }
    return *this;
}

FixtureGroup &FixtureGroup::operator=(FixtureGroup &&other)
{
    if (this != &other)
    {
        const uint64_t rev = m_revision;
        m_name = std::move(other.m_name);
        m_fixtures = std::move(other.m_fixtures);
        m_usedUniverses = std::move(other.m_usedUniverses);
        m_cacheDirty = true;
        m_revision = rev + 1;
    }
    return *this;
}

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

void FixtureGroup::rebuildCache() const
{
    m_byAttribute.clear();
    m_colorCells.clear();
    m_fids.clear();
    m_fids.reserve(m_fixtures.size());
    for (const auto &f : m_fixtures)
    {
        m_fids.push_back(f->Fid());
        for (const auto &[attr, params] : f->ByAttribute())
        {
            auto &bucket = m_byAttribute[attr];
            bucket.insert(bucket.end(), params.begin(), params.end());
        }
        for (auto &cell : f->ColorCells())
        {
            m_colorCells.push_back(&cell);
        }
    }
    m_cacheDirty = false;
}

bool FixtureGroup::contains(const FixturePtr &fixture) const
{
    return std::find(m_fixtures.begin(), m_fixtures.end(), fixture) !=
           m_fixtures.end();
}

void FixtureGroup::add(const FixturePtr &fixture)
{
    if (fixture == nullptr || contains(fixture))
    {
        return;
    }
    m_fixtures.push_back(fixture);
    // m_usedUniverses.insert(fixture->Universe());
    m_cacheDirty = true;
    ++m_revision;
}

void FixtureGroup::add(const std::vector<FixturePtr> &fixtures)
{
    for (const auto &f : fixtures)
    {
        add(f);
    }
}

void FixtureGroup::add(const FixtureGroup &other) { add(other.m_fixtures); }

void FixtureGroup::clear()
{
    m_fixtures.clear();
    m_usedUniverses.clear();
    m_byAttribute.clear();
    m_colorCells.clear();
    m_fids.clear();
    m_cacheDirty = true;
    ++m_revision;
}

const std::vector<FixtureGroup::FixturePtr> &FixtureGroup::fixtures() const
{
    return m_fixtures;
}

const std::vector<uint16_t> &FixtureGroup::fids() const
{
    if (m_cacheDirty)
    {
        rebuildCache();
    }
    return m_fids;
}

std::set<uint16_t> FixtureGroup::fidsSet() const
{
    std::set<uint16_t> ret;
    for (auto &fid : m_fids)
    {
        ret.insert(fid);
    }

    return ret;
}

std::size_t FixtureGroup::fidsHash() const
{
    std::vector<uint16_t> ids = fids(); // a copy: fids() is the live cache
    std::sort(ids.begin(), ids.end());

    // FNV-1a over the sorted fid bytes.
    std::size_t h = 1469598103934665603ULL;
    for (const uint16_t fid : ids)
    {
        h ^= static_cast<std::size_t>(fid & 0xFF);
        h *= 1099511628211ULL;
        h ^= static_cast<std::size_t>((fid >> 8) & 0xFF);
        h *= 1099511628211ULL;
    }
    return h;
}

const std::vector<Fixtures::Parameter *> &
FixtureGroup::parameters(GDTF::Attribute attr) const
{
    static const std::vector<Fixtures::Parameter *> empty;
    if (m_cacheDirty)
    {
        rebuildCache();
    }
    const auto it = m_byAttribute.find(attr);
    return it != m_byAttribute.end() ? it->second : empty;
}

bool FixtureGroup::has(GDTF::Attribute attr) const
{
    if (m_cacheDirty)
    {
        rebuildCache();
    }
    return m_byAttribute.contains(attr);
}

const std::vector<Fixtures::ColorCell *> &FixtureGroup::colorCells() const
{
    if (m_cacheDirty)
    {
        rebuildCache();
    }
    return m_colorCells;
}

FixtureGroup &FixtureGroup::operator+=(const FixturePtr &fixture)
{
    add(fixture);
    return *this;
}

FixtureGroup &FixtureGroup::operator+=(const FixtureGroup &other)
{
    add(other);
    return *this;
}

FixtureGroup &FixtureGroup::operator+=(const std::vector<FixturePtr> &fixs)
{
    add(fixs);
    return *this;
}

std::string FixtureGroup::describe() const
{
    std::string s = "Group \"" + m_name + "\" [" +
                    std::to_string(m_fixtures.size()) + " fixtures, " +
                    std::to_string(m_usedUniverses.size()) + " universes]";
    for (const auto &f : m_fixtures)
    {
        s += "\n  - " + f->Name() + " (fid " + std::to_string(f->Fid()) + ")";
    }
    return s;
}
} // namespace LightEngine::DMX
