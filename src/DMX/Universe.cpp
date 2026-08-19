#include "LightEngine/DMX/Universe.h"

#include "Utils/Colors/Colors.h"
#include "Utils/Colors/Font.h"
#include "Utils/Colors/HSV.h"

#include <cmath>
#include <iomanip>
#include <iterator>
#include <set>
#include <sstream>

namespace LightEngine::DMX
{
using Fixture = LightEngine::Fixtures::Fixture;

// Identify fixtures inserted by a base add(): snapshot the current fragments,
// run the add, then return the ones that are new. The base storage inserts in
// sorted order and gives us no handle otherwise.
namespace
{
std::set<const Fixture *>
snapshot(const std::list<std::shared_ptr<Fixture>> &frags)
{
    std::set<const Fixture *> s;
    for (const auto &f : frags)
    {
        s.insert(f.get());
    }
    return s;
}
} // namespace

void Universe::wireBuffers()
{
    // The buffer address (getBytes()) is stable for the universe's lifetime, so
    // re-pointing every fixture after each add is safe and keeps newly-placed
    // fixtures and their parameters writing in-bounds.
    for (const auto &f : m_fragments)
    {
        f->setBuffer(getBytes());
        // f->SetUniverse(m_id);
    }
}

void Universe::reindex()
{
    m_byName.clear();
    for (const auto &f : m_fragments)
    {
        m_byName[f->Name()].push_back(f);
    }
}

Universe::FixturePtr Universe::addFixture(const Fixture &fixture)
{
    const auto before = snapshot(m_fragments);
    add(fixture);
    wireBuffers();
    reindex();
    for (const auto &f : m_fragments)
    {
        if (!before.contains(f.get()))
        {
            return f;
        }
    }
    return nullptr;
}

Universe::FixturePtr Universe::addFixture(const Fixture &fixture,
                                          uint32_t start)
{
    const auto before = snapshot(m_fragments);
    add(fixture, start);
    wireBuffers();
    reindex();
    for (const auto &f : m_fragments)
    {
        if (!before.contains(f.get()))
        {
            return f;
        }
    }
    return nullptr;
}

std::vector<Universe::FixturePtr>
Universe::addFixtures(const Fixture &fixture, int count, uint32_t start)
{
    const auto before = snapshot(m_fragments);
    addMultiple(fixture, count, static_cast<int>(start));
    wireBuffers();
    reindex();
    std::vector<FixturePtr> placed;
    for (const auto &f : m_fragments)
    {
        if (!before.contains(f.get()))
        {
            placed.push_back(f);
        }
    }
    return placed;
}

Universe::FixturePtr Universe::at(std::size_t index) const
{
    if (index >= m_fragments.size())
    {
        return nullptr;
    }
    return *std::next(m_fragments.begin(), static_cast<long>(index));
}

const std::vector<Universe::FixturePtr> &
Universe::byName(const std::string &name) const
{
    static const std::vector<FixturePtr> empty;
    const auto it = m_byName.find(name);
    return it != m_byName.end() ? it->second : empty;
}

std::string Universe::describe() const
{
    return "Universe " + std::to_string(m_id) + "\n" + fragmentsToString();
}

std::string Universe::dump(int channels) const
{
    namespace F = Utils::Font;

    // map every channel to the index of the fixture that owns it (-1 =
    // unpatched)
    std::vector<int> owner(512, -1);
    int idx = 0;
    for (const auto &f : m_fragments)
    {
        for (uint32_t c = 0; c < f->Footprint(); ++c)
        {
            owner[f->start + c] = idx;
        }
        ++idx;
    }

    auto colorFor = [](int ownerIdx) -> std::string
    {
        if (ownerIdx < 0)
        {
            return F::colorDim;
        }
        // distinct hue per fixture
        Utils::Colors::HSV hsv(std::fmod(110.f + ownerIdx * 65.f, 360.f), 0.5f,
                               0.9f);
        return F::colorByRGB(Utils::Colors::hsvToRgb(hsv), true);
    };

    const int cols = 16;
    std::ostringstream ss;
    ss << "Universe " << m_id << "  (" << m_fragments.size() << " fixtures)\n"
       << F::colorItalic;
    for (int i = 0; i < cols; ++i)
    {
        ss << "0x" << "0123456789ABCDEF"[i] << " ";
    }
    ss << F::colorReset << "\n";

    auto sep = [&]
    {
        for (int i = 0; i < cols; ++i)
            ss << "----";
        ss << "\n";
    };
    sep();

    for (int i = 0; i < channels; ++i)
    {
        if (i % cols == 0 && i != 0)
        {
            ss << "\n";
        }
        ss << colorFor(owner[i]) << std::setw(3) << int(m_buffer[i])
           << F::colorReset << " ";
    }
    ss << "\n";
    sep();
    return ss.str();
}
} // namespace LightEngine::DMX
