#pragma once
#include <memory>

#include "LightEngine/Effects/Static/StaticSnapshot.h"
#include "PoolObject.h"

//
// Preset: a stored value snapshot - what `store preset color`/`store preset
// dimmer` produce. One class for every attribute type; which one a given
// preset holds is carried by its StaticSnapshot's own EffectCategory rather
// than by a separate Pools::Color / Pools::Dimmer class per type.
//
namespace LightEngine::Engine::Pools
{
class Preset : public PoolObject
{
    std::shared_ptr<Effects::StaticSnapshot> m_effect;

public:
    explicit Preset(std::shared_ptr<Effects::StaticSnapshot> effect)
        : m_effect(std::move(effect))
    {
    }

    [[nodiscard]] const std::shared_ptr<Effects::StaticSnapshot> &effect() const
    {
        return m_effect;
    }

    [[nodiscard]] std::string toString() const override
    {
        // nameExpr is pre-formatted to a plain string rather than spliced in
        // as a literal - see Pools::Group::toString for why: an unnamed
        // object's "Unnamed" literal has no {}, which would otherwise shift
        // the fixture count argument into the empty name() slot instead.
        const std::string nameExpr =
            name().empty() ? Theme::dim("Unnamed") : Utils::Font::format(Theme::txt("{}"), name());

        return Utils::Font::format(
            Utils::Font::group(Theme::lbl("Preset "), "[", Theme::num("{}"), "]: ", "{}",
                               " ({} fixtures)"),
            number(), nameExpr, m_effect->values().size());
    }
};
} // namespace LightEngine::Engine::Pools
