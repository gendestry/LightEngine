#pragma once

#include <cstdint>
#include <unordered_map>
#include <utility>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Effects/EffectSpec.h"
#include "LightEngine/Engine/TimeContext.h"
#include "LightEngine/Fixture/Fixture.h"

//
// Effect: a unit of behaviour re-evaluated every frame. It OWNS its target
// FixtureGroup by value (a cheap bag of shared fixture references), so a
// factory-built effect is fully self-contained.
//
// Pull model: an effect is a *pure function* valueFor(fid, t, base) - it has no
// per-frame mutable state and never writes to fixtures. The ProgrammerLayer
// queries it while composing the Frame, passing the fixture's already-resolved
// base value so a relative effect can modulate it (and an absolute one ignore
// it). spec() reports the current recipe for storing into a cue.
//
namespace LightEngine::Effects
{
class Effect
{
protected:
    Spec m_spec;
    DMX::FixtureGroup m_group; // owned by value; order defines the spread
    std::unordered_map<uint16_t, std::size_t> m_index; // fid -> position

    // Rebuild the fid -> position map from the group's order. Call from the
    // ctor and whenever the group changes.
    void reindex()
    {
        m_index.clear();
        const auto &fx = m_group.fixtures();
        for (std::size_t i = 0; i < fx.size(); ++i)
            m_index[fx[i]->Fid()] = i;
    }

public:
    Effect(Spec spec, DMX::FixtureGroup group)
        : m_spec(std::move(spec)), m_group(std::move(group))
    {
        reindex();
    }
    virtual ~Effect() = default;

    [[nodiscard]] Target target() const { return targetOf(m_spec.kind); }
    [[nodiscard]] const Spec &spec() const { return m_spec; }
    [[nodiscard]] const DMX::FixtureGroup &group() const { return m_group; }
    [[nodiscard]] DMX::FixtureGroup &group() { return m_group; }

    // The whole interface. `base` is the fixture's resolved value this frame
    // (from the static edits below the effect); relative effects modulate it,
    // absolute effects ignore it. Returns an unset Sample for a non-target fid.
    [[nodiscard]] virtual Sample valueFor(uint16_t fid,
                                          const Engine::TimeContext &t,
                                          const Sample &base) const = 0;
};
} // namespace LightEngine::Effects
