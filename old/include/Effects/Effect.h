#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Effects/EffectSpec.h"
#include "LightEngine/Engine/Layers/Frame.h"
#include "LightEngine/Engine/TimeContext.h"

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
class Effect
{
protected:
    DMX::FixtureGroup m_group;
    bool              m_enabled = true;

    // FIDs of the fixtures this effect targets (for building a Spec).
    [[nodiscard]] std::vector<uint16_t> targetFids() const { return m_group.fids(); }

public:
    explicit Effect(DMX::FixtureGroup group) : m_group(std::move(group)) {}
    virtual ~Effect() = default;

    // The bridge to the frame model: build FixtureValues and contribute() them,
    // instead of mutating fixtures directly.
    virtual void apply(Engine::Frame& frame, const Engine::TimeContext& t) = 0;
    [[nodiscard]] virtual Spec spec() const = 0;

    void setEnabled(bool enabled) { m_enabled = enabled; }
    [[nodiscard]] bool enabled() const { return m_enabled; }
    [[nodiscard]] DMX::FixtureGroup& group() { return m_group; }
    [[nodiscard]] const DMX::FixtureGroup& group() const { return m_group; }
};
} // namespace LightEngine::Effects
