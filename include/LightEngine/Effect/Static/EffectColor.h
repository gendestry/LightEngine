#pragma once

#include <utility>

#include "Utils/Colors/RGB.h"
// #include ""

#include "LightEngine/Effect/EffectBase.h"

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
    void setColor(Utils::Colors::RGB color) { m_color = color; }
    void apply(Engine::Frame &frame, const Engine::TimeContext &t,
               const DMX::FixtureGroup &group) override;

    EFFECT_CATEGORY(COLOR);
    // [[nodiscard]] Spec spec() const override;
};

} // namespace LightEngine::Effects