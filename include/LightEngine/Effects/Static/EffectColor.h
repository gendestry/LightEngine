#pragma once

#include <utility>

#include "Utils/Colors/RGB.h"

#include "LightEngine/Effects/Static/EffectBaseStatic.h"

//
// Concrete effects. Static ones ignore time; dynamic ones animate off the
// TimeContext. Per-fixture effects use the fixture's index within the group.
// Each effect owns its group (passed by value) and contributes into the frame.
//
namespace LightEngine::Effects
{
// Holds the group at a constant colour (a "set colour" is just this).
class StaticColor : public EffectStatic
{
    Utils::Colors::RGB m_color;

public:
    StaticColor(Utils::Colors::RGB color) : m_color(color) {}
    void setColor(Utils::Colors::RGB color)
    {
        m_color = color;
        markDirty();
    }

protected:
    void recompute(const Utils::Time::TimeContext &t,
                   const Utils::Maths::Interval &group) override;

public:
    EFFECT_CATEGORY(COLOR);
    EFFECT_CLONE(StaticColor);
};

} // namespace LightEngine::Effects