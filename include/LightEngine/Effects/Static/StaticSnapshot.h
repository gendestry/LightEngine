#pragma once

#include <map>
#include <utility>
#include <vector>

#include "LightEngine/Effects/Static/EffectBaseStatic.h"
#include "Utils/Math/Interval.h"

//
// StaticSnapshot: what a preset actually stores - a per-fixture value table,
// captured once at store time. Unlike a live static effect (StaticColor,
// StaticIntensity), it has no parameter of its own to re-derive values from;
// it just replays exactly what it was given.
//
// recompute() intersects the table against whatever group it's handed, so the
// same snapshot self-masks on recall: pushed with a subset of its own
// fixtures selected, it emits only the entries that fall inside that subset.
//
// EFFECT_CATEGORY is deliberately not used here: it's static-per-class, but
// one snapshot serves both a color preset and a dimmer preset depending on
// which attribute was captured, so the category is carried as instance state
// instead and set at construction.
//
namespace LightEngine::Effects
{
class StaticSnapshot : public EffectStatic
{
    std::map<uint16_t, Engine::FixtureValues> m_values;
    EffectCategory m_category;

public:
    StaticSnapshot(const std::vector<std::pair<uint16_t, Engine::FixtureValues>> &values,
                   EffectCategory category)
        : m_values(values.begin(), values.end()), m_category(category)
    {
    }

    [[nodiscard]] const std::map<uint16_t, Engine::FixtureValues> &values() const
    {
        return m_values;
    }

    // The fixtures this snapshot has a value for, as an Interval - the
    // preset's own coverage, independent of whatever group it's recalled onto.
    [[nodiscard]] Utils::Maths::Interval coverage() const;

protected:
    void recompute(const Utils::Time::TimeContext &t,
                   const Utils::Maths::Interval &group) override;

public:
    EffectCategory GetEffectCategory() const override { return m_category; }
    const char *GetCategoryName() const override
    {
        return m_category == EffectCategory::COLOR ? "COLOR" : "DIMMER";
    }

    EFFECT_CLONE(StaticSnapshot);
};
} // namespace LightEngine::Effects
