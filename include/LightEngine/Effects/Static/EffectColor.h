#pragma once

#include <utility>

#include "Utils/Colors/RGB.h"
// #include ""

#include "LightEngine/Effects/Static/EffectBaseStatic.h"

//
// Concrete effects. Static ones ignore time; dynamic ones animate off the
// TimeContext. Per-fixture effects use the fixture's index within the group.
// Each effect owns its group (passed by value), contributes into the frame and
// can report its Spec.
//
namespace LightEngine::Effects
{
// Holds the group at a constant colour (a "set colour" is just this).
class StaticColor : public EffectStatic
{
    Utils::Colors::RGB m_color;

public:
    StaticColor(Utils::Colors::RGB color) : m_color(color) {}
    StaticColor(
        const std::vector<std::pair<uint16_t, LightEngine::Engine::FixtureValues>> &vals)
        : EffectStatic(vals)
    {
    }
    void setColor(Utils::Colors::RGB color)
    {
        m_color = color;
        markDirty();
    }

    // virtual void setCache(
    //     const std::vector<std::pair<uint16_t, LightEngine::Engine::FixtureValues>> &vals)
    //     override
    // {
    //     m_cache = vals;
    // }

protected:
    void recompute(const LightEngine::Engine::TimeContext &t,
                   const DMX::FixtureGroup &group) override;

public:
    EFFECT_CATEGORY(COLOR);

    // std::string describe() const override
    // {
    //     auto &c = m_color;
    //     return std::format("Set color to {} {} {}", c.r, c.g, c.b);
    // };
    // [[nodiscard]] Spec spec() const override;
};

} // namespace LightEngine::Effects