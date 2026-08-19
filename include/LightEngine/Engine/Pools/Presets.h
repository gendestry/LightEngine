#pragma once
#include <string>

#include "Utils/Colors/Font.h"

#include "LightEngine/Engine/Pools/Color.h"
#include "LightEngine/Engine/Pools/Group.h"
#include "LightEngine/Engine/Pools/Pool.h"
// #include "LightEngine/Engine/Pools/Sequence.h"  // TODO (cues live inside)
// #include "LightEngine/Engine/Pools/Effect.h"    // TODO

//
// Stored: the show's object storage - one Pool per object type, owned together
// by the Engine. This is the single home for everything the user builds and
// numbers (groups, presets, cues/sequences, effects...). The Engine reaches
// through here rather than owning a loose pool per type.
//
namespace LightEngine::Engine
{
class Presets
{
    Pool<Pools::Group> m_groups;
    Pool<Pools::Color> m_colors;
    // Pool<Pools::DimmerPreset> m_dimmerPresets;
    // Pool<Pools::Sequence> m_sequences;  // TODO
    // Pool<Pools::Effect>   m_effects;    // TODO

public:
    Presets() : m_groups("Groups"), m_colors("Colors") {}
    [[nodiscard]] Pool<Pools::Group> &groups() { return m_groups; }
    [[nodiscard]] const Pool<Pools::Group> &groups() const { return m_groups; }

    [[nodiscard]] Pool<Pools::Color> &colors() { return m_colors; }
    [[nodiscard]] const Pool<Pools::Color> &colors() const { return m_colors; }

    // [[nodiscard]] Pool<Pools::DimmerPreset> &dimmerPresets()
    // {
    //     return m_dimmerPresets;
    // }
    // [[nodiscard]] const Pool<Pools::DimmerPreset> &dimmerPresets() const
    // {
    //     return m_dimmerPresets;
    // }

    void clear()
    {
        m_groups.clear();
        m_colors.clear();
        // m_dimmerPresets.clear();
    }

    // Multi-line dump of every pool, each under a labelled header.
    [[nodiscard]] std::string describe() const
    {
        std::string s = Utils::Font::bold + "Stored" + Utils::Font::reset;
        s += section("Groups", m_groups);
        s += section("Colors", m_colors);
        // s += section("DimmerPresets", m_dimmerPresets);
        return s;
    }

private:
    // One labelled pool section: "\n<yellow>Label<reset> <pool dump>".
    template <class P>
    static std::string section(const std::string &label, const P &pool)
    {
        return "\n" + Utils::Font::colorYellow + label + Utils::Font::reset +
               " " + pool.describe();
    }
};
} // namespace LightEngine::Engine
