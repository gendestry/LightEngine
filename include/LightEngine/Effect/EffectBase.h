#pragma once

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "LightEngine/DMX/FixtureGroup.h"
// #include "LightEngine/Effects/EffectSpec.h"
#include "LightEngine/Engine/Layers/Frame.h"
#include "LightEngine/Engine/TimeContext.h"
#include "Utils/Math/Curve.h"

//
// Effect: a unit of behaviour re-evaluated every frame. It OWNS its target
// FixtureGroup by value (a cheap bag of shared fixture references), so a
// factory-built effect is fully self-contained - no external group storage.
//
// Unlike the original (fixture-mutating) design, an effect writes into the
// per-frame Frame via contribute(); fixtures hold no state in this engine.
// spec() reports its current description (for storing into a cue).
//
namespace LightEngine::Effects
{
enum class EffectType
{
    STATIC,
    ANIMATED
};

enum class EffectCategory
{
    DIMMER,
    COLOR
};

#define EFFECT_CATEGORY(type)                                                  \
    static EffectCategory GetStaticCategory() { return EffectCategory::type; } \
    virtual EffectCategory GetEffectCategory() const override                  \
    {                                                                          \
        return GetStaticCategory();                                            \
    }                                                                          \
    virtual const char *GetCategoryName() const override { return #type; }

class EffectBase
{
protected:
    // DMX::FixtureGroup m_group;
    bool m_enabled = true;
    EffectType m_type;

    // FIDs of the fixtures this effect targets (for building a Spec).
    // [[nodiscard]] std::vector<uint16_t> targetFids() const { return
    // m_group.fids(); }

public:
    EffectBase(EffectType type) : m_type(type) {}
    // explicit EffectBase(DMX::FixtureGroup group) : m_group(std::move(group))
    // {}
    virtual ~EffectBase() = default;

    // The bridge to the frame model: build FixtureValues and contribute() them,
    // instead of mutating fixtures directly.
    virtual void apply(Engine::Frame &frame, const Engine::TimeContext &t,
                       const DMX::FixtureGroup &group) = 0;

    EffectType getType() const { return m_type; }
    // [[nodiscard]] virtual Spec spec() const = 0;

    void setEnabled(bool enabled) { m_enabled = enabled; }
    [[nodiscard]] bool enabled() const { return m_enabled; }

    virtual EffectCategory GetEffectCategory() const = 0;
    virtual const char *GetCategoryName() const = 0;
    // [[nodiscard]] DMX::FixtureGroup& group() { return m_group; }
    // [[nodiscard]] const DMX::FixtureGroup& group() const { return m_group; }
}; // namespace LightEngine::Effects

class EffectStatic : public EffectBase
{
public:
    EffectStatic() : EffectBase(EffectType::STATIC) {}

    // EFFECT_TYPE(STATIC);
};

class EffectAnimated : public EffectBase
{
protected:
    std::unique_ptr<Utils::Maths::Curve> m_curve;
    float m_bpm;
    float m_spread;

public:
    EffectAnimated() : EffectBase(EffectType::ANIMATED) {}
    void setBpm(float bpm) { m_bpm = bpm; }
    [[nodiscard]] float bpm() const { return m_bpm; }
    void setSpread(float spread) { m_spread = spread; }
    [[nodiscard]] float spread() const { return m_spread; }
};
} // namespace LightEngine::Effects
