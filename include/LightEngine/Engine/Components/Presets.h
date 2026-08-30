#pragma once
#include <string>

#include "Utils/Colors/Font.h"

#include "LightEngine/Engine/Pools/Group.h"
#include "LightEngine/Engine/Pools/Pool.h"
#include "LightEngine/Engine/Pools/Preset.h"
#include "Utils/Logging/Logger.h"
// #include "LightEngine/Engine/Pools/Sequence.h"  // TODO (cues live inside)
// #include "LightEngine/Engine/Pools/Effect.h"    // TODO

//
// Presets: the show's object storage - one Pool per object type, owned
// together by the Engine. This is the single home for everything the user
// builds and numbers (groups, presets, cues/sequences, effects...). The
// Engine reaches through here rather than owning a loose pool per type.
//
// One Pool<Preset> per attribute (color, dimmer, ...) rather than one member
// variable per preset type - adding a new preset type is a new
// EffectCategory case, not a new member + accessor + clear() line.
//
namespace LightEngine::Engine::Components
{
class Presets : public Utils::Traits::Stringify
{
    Pool<Pools::Group> m_groups;
    std::map<Effects::EffectCategory, Pool<Pools::Preset>> m_presets;
    // Pool<Pools::Sequence> m_sequences;  // TODO
    // Pool<Pools::Effect>   m_effects;    // TODO

public:
    Presets() : m_groups("Groups")
    {
        m_presets.emplace(Effects::EffectCategory::COLOR, Pool<Pools::Preset>("Colors"));
        m_presets.emplace(Effects::EffectCategory::DIMMER, Pool<Pools::Preset>("Dimmers"));
    }

    [[nodiscard]] Pool<Pools::Group> &groups() { return m_groups; }
    [[nodiscard]] const Pool<Pools::Group> &groups() const { return m_groups; }

    [[nodiscard]] Pool<Pools::Preset> &presets(Effects::EffectCategory cat)
    {
        return m_presets.at(cat);
    }
    [[nodiscard]] const Pool<Pools::Preset> &presets(Effects::EffectCategory cat) const
    {
        return m_presets.at(cat);
    }

    void clear()
    {
        m_groups.clear();
        for (auto &[cat, pool] : m_presets)
        {
            pool.clear();
        }
    }

    // Multi-line dump of every pool, each under a labelled header.
    [[nodiscard]] std::string toString() const override
    {
        std::string s = Utils::Font::bold + "Stored" + Utils::Font::reset;
        s += section("Groups", m_groups);
        for (const auto &[cat, pool] : m_presets)
        {
            s += section(pool.name(), pool);
        }
        return s;
    }

private:
    // One labelled pool section: "\n<yellow>Label<reset> <pool dump>".
    template <class P>
    static std::string section(const std::string &label, const P &pool)
    {
        return Utils::Font::format(Utils::Font::group("\n", Theme::accent("{} "), "{}"), label, pool.toString());
    }
};
} // namespace LightEngine::Engine::Components
