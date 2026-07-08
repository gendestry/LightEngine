#include "LightEngine/DMX/FixtureGroup.h"

#include <algorithm>
#include <utility>

#include "Utils/Colors/Colors.h"

namespace LightEngine::DMX
{
FixtureGroup::FixtureGroup(std::string name) : m_name(std::move(name)) {}

void FixtureGroup::rebuildCache() const
{
    m_byAttribute.clear();
    m_colorCells.clear();
    for (const auto &f : m_fixtures)
    {
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
    m_usedUniverses.insert(fixture->Universe());
    m_cacheDirty = true;
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
    m_cacheDirty = true;
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

void FixtureGroup::setColor(const Utils::Colors::HSV &hsv)
{
    for (const auto &f : m_fixtures)
    {
        f->SetColor(hsv);
    }
}

void FixtureGroup::setColor(const Utils::Colors::RGB &rgb)
{
    setColor(Utils::Colors::rgbToHsv(rgb));
}

void FixtureGroup::setIntensity(float v)
{
    for (const auto &f : m_fixtures)
    {
        f->SetIntensity(v);
    }
}

void FixtureGroup::resolve()
{
    for (const auto &f : m_fixtures)
    {
        f->Resolve();
    }
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
