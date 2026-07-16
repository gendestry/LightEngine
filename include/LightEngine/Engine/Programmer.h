#pragma once
#include <cstdint>
#include <map>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Effects/EffectEngine.h"
#include "LightEngine/Effects/EffectSpec.h"
#include "LightEngine/Engine/Frame.h"
#include "LightEngine/Engine/Layer.h"
#include "LightEngine/Engine/Patch.h"
#include "LightEngine/Engine/TimeContext.h"

//
// ProgrammerLayer: the live editing layer, composed in two passes.
//
//   1. static  - absolute per-fixture edits (setColor/setIntensity/...). These
//                are the console "hard values": what you dialled in by hand.
//   2. dynamic - running Effects (DimmerChase, ColorFade, ...). Pull model: each
//                effect is a pure valueFor(fid, time, base) queried here while
//                composing the Frame. Effects ride ON TOP of the static base for
//                their target attribute, so `group 1 at 100` + a DimmerChase
//                gives a wave whose ceiling is the static 100%.
//
// Both passes contribute LTP into the Frame; the dynamic pass runs last so a
// running effect wins over the static value it modulates.
//
namespace LightEngine::Engine
{

class ProgrammerLayer : public Layer
{
    // The live selection: transient, ordered, cached. NOT a Pools::Group - that
    // is a stored object; this is "what I'm editing right now". Engine
    // snapshots it into a Pools::Group on storeGroup().
    Patch &m_patch; // resolves raw FIDs -> live fixtures for selection
    DMX::FixtureGroup m_selection;
    std::map<uint16_t, FixtureValues> m_edits; // FID -> static (hard) values
    Effects::EffectEngine m_effects;           // owns the running dynamic effects

    Utils::Colors::HSV &ensureColor(uint16_t fid)
    {
        FixtureValues &v = m_edits[fid];
        if (!v.color)
            v.color = Utils::Colors::HSV{0.f, 0.f, 0.f};
        return *v.color;
    }

    // The static base an effect modulates for `fid`: its hard-dialled value, or
    // an unset Sample if the fixture has no static edit.
    [[nodiscard]] Effects::Sample baseSample(uint16_t fid) const
    {
        Effects::Sample s;
        const auto it = m_edits.find(fid);
        if (it != m_edits.end() && it->second.color)
        {
            s.level = it->second.color->v; // intensity == HSV.v
            s.color = *it->second.color;
        }
        return s;
    }

public:
    explicit ProgrammerLayer(Patch &patch) : m_patch(patch) {}

    // ---- selection ----
    // select() replaces the current selection, add() accumulates onto it.
    // FixtureGroup overloads take an already-resolved selection; the FID
    // overloads resolve through the patch (skips unpatched FIDs).
    void select(const DMX::FixtureGroup &g)
    {
        m_selection.clear();
        m_selection.add(g);
    }
    void select(const std::vector<uint16_t> &fids)
    {
        m_selection.clear();
        m_selection.add(m_patch.getFixtures(fids));
    }
    void add(const DMX::FixtureGroup &g) { m_selection += g; }
    void add(const std::vector<uint16_t> &fids)
    {
        m_selection.add(m_patch.getFixtures(fids));
    }
    void deselect() { m_selection.clear(); }
    [[nodiscard]] const DMX::FixtureGroup &selection() const
    {
        return m_selection;
    }

    // ---- static edits (scoped to the current selection) ----
    void setColor(const Utils::Colors::HSV &hsv)
    {
        for (const auto &f : m_selection.fixtures())
            ensureColor(f->Fid()) = hsv;
    }
    void setHueSat(float h, float s)
    {
        for (const auto &f : m_selection.fixtures())
        {
            Utils::Colors::HSV &c = ensureColor(f->Fid());
            c.h = h;
            c.s = s;
        }
    }
    void setIntensity(float v)
    {
        for (const auto &f : m_selection.fixtures())
            ensureColor(f->Fid()).v = v;
    }
    // Distribute intensity a..b across the selection in order (t = i/(n-1)).
    void setIntensityRamp(float a, float b)
    {
        const auto &fx = m_selection.fixtures();
        const std::size_t n = fx.size();
        for (std::size_t i = 0; i < n; ++i)
        {
            const float t = n <= 1 ? 0.f : float(i) / float(n - 1);
            ensureColor(fx[i]->Fid()).v = a + (b - a) * t;
        }
    }
    // Fan hue hueA..hueB across the selection in order, at fixed saturation.
    void fanColor(float hueA, float hueB, float sat = 1.f)
    {
        const auto &fx = m_selection.fixtures();
        const std::size_t n = fx.size();
        for (std::size_t i = 0; i < n; ++i)
        {
            const float t = n <= 1 ? 0.f : float(i) / float(n - 1);
            Utils::Colors::HSV &c = ensureColor(fx[i]->Fid());
            c.h = hueA + (hueB - hueA) * t;
            c.s = sat;
        }
    }

    // Per-fixture setters used by preset recall (bypass the selection).
    void applyHueSat(uint16_t fid, float h, float s)
    {
        Utils::Colors::HSV &c = ensureColor(fid);
        c.h = h;
        c.s = s;
    }
    void applyIntensity(uint16_t fid, float v) { ensureColor(fid).v = v; }
    [[nodiscard]] const std::map<uint16_t, FixtureValues> &edits() const
    {
        return m_edits;
    }

    // ---- dynamic effects ----
    // Attach a running effect built from `spec`, bound to a group. The no-group
    // overload uses the current selection, so `select -> addEffect` reads like
    // the console. Returns a non-owning pointer (the EffectEngine owns it), or
    // nullptr if the kind has no concrete effect yet.
    Effects::Effect *addEffect(const Effects::Spec &spec)
    {
        return m_effects.build(spec, m_selection);
    }
    Effects::Effect *addEffect(const Effects::Spec &spec,
                               const DMX::FixtureGroup &group)
    {
        return m_effects.build(spec, group);
    }
    void removeEffect(Effects::Effect *fx) { m_effects.remove(fx); }
    void clearEffects() { m_effects.clear(); }
    [[nodiscard]] const Effects::EffectEngine &effects() const
    {
        return m_effects;
    }

    // ---- clear (staged, console-style) ----
    void clearValues() // keep selection, drop static values AND effects
    {
        m_edits.clear();
        m_effects.clear();
    }
    void clearAll() // wipe everything
    {
        m_selection.clear();
        m_edits.clear();
        m_effects.clear();
    }

    int priority() const override { return 1000; } // programmer wins
    void apply(Frame &frame, const TimeContext &time) override
    {
        // 1. static pass: absolute programmer edits.
        for (const auto &[fid, values] : m_edits)
            frame.contribute(fid, values, MergePolicy::LTP);

        // 2. dynamic pass: effects modulate/override the static base, per
        //    target attribute. Runs after the static pass so it wins (LTP).
        for (const auto &fx : m_effects.effects())
        {
            const Effects::Target tgt = fx->target();
            for (const auto &f : fx->group().fixtures())
            {
                const uint16_t fid = f->Fid();
                const Effects::Sample base = baseSample(fid);
                const Effects::Sample s = fx->valueFor(fid, time, base);

                FixtureValues fv;
                if (tgt == Effects::Target::Intensity && s.level)
                {
                    // Override intensity only; keep the static hue/sat so a
                    // dimmer chase never disturbs the programmed colour.
                    Utils::Colors::HSV c =
                        base.color.value_or(Utils::Colors::HSV{0.f, 0.f, 0.f});
                    c.v = *s.level;
                    fv.color = c;
                }
                else if (tgt == Effects::Target::Color && s.color)
                {
                    fv.color = *s.color;
                }
                else
                {
                    continue; // effect does not drive this fid this frame
                }
                frame.contribute(fid, fv, MergePolicy::LTP);
            }
        }
    }

    // A copy of the live selection - Engine wraps this into a Pools::Group on
    // store. Keeps the programmer free of any pool-object dependency.
    [[nodiscard]] DMX::FixtureGroup selectedGroup() const { return m_selection; }
};
} // namespace LightEngine::Engine
