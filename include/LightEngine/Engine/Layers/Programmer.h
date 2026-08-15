#pragma once
#include <cstdint>

#include "LightEngine/DMX/FixtureGroup.h"
// #include "LightEngine/Effects/EffectFactory.h"
#include "LightEngine/Engine/Layers/Frame.h"
#include "LightEngine/Engine/Layers/Layer.h"
#include "LightEngine/Engine/Patch.h"
#include "LightEngine/Engine/TimeContext.h"

//
// Layer: anything that produces values for a frame. Programmer, playback and
// effects are all just layers. update() composes them low priority -> high, so
// a higher-priority layer's LTP writes land on top of a lower one's.
//
namespace LightEngine::Engine
{

// struct StaticEffectPool
// {
// };
//
// The live editing layer - what the CommandBuilder/CLI drives. Holds the user's
// edits per fixture; a selection scopes the bulk setters.
class Programmer : public Layer
{
    // The live selection: transient, ordered, cached. NOT a Pools::Group - that
    // is a stored object; this is "what I'm editing right now". Engine
    // snapshots it into a Pools::Group on storeGroup().
    Patch &m_patch; // resolves raw FIDs -> live fixtures for selection
    // Effects::EffectHolder running;

    // std::map<uint16_t, FixtureValues> m_edits; // FID -> touched values

    Utils::Colors::HSV &ensureColor(uint16_t fid);

    [[nodiscard]] DMX::FixtureGroup singleFixture(uint16_t fid) const;
    // void push(std::unique_ptr<Effects::Effect> e)
    // {
    //     running.c_ptr->effects.push_back(std::move(e));
    //     running.c_ptr->dirty = true;
    // }

public:
    explicit Programmer(Patch &patch) : Layer(Priority::PROG), m_patch(patch) {}

    // ---- selection ----
    // select() replaces the current selection, add() accumulates onto it.
    // FixtureGroup overloads take an already-resolved selection; the FID
    // overloads resolve through the patch (skips unpatched FIDs).
    void select(const DMX::FixtureGroup &g);
    void select(const std::vector<uint16_t> &fids);
    void add(const DMX::FixtureGroup &g);
    void add(const std::vector<uint16_t> &fids);
    // The live selection is the current stack's selection.
    // [[nodiscard]] const DMX::FixtureGroup &selection() const
    // {
    // return running.c_ptr->selection;
    // }

    // ---- edits (scoped to the current selection) ----
    // void setColor(const Utils::Colors::HSV &hsv); // sets hue/sat only
    void setColor(const Utils::Colors::RGB &rgb); // converts to HSV, hue/sat
    // void setColorGradient(const Utils::Colors::RGB &rgb,
    //                       const Utils::Colors::RGB &rgb2);
    // void setHueSat(float h, float s);
    void setIntensity(float v);

    // void applyHueSat(uint16_t fid, float h, float s);
    // void applyIntensity(uint16_t fid, float v);

    // A dimmer chase across the current selection. `resolution` is both the
    // curve's sample count and the effect's step grain: it recomputes that many
    // times per beat, not once per frame.
    // void setIntensityRamp(Utils::Maths::Type curve = Utils::Maths::SINUSOID,
    //                       float bpm = 60.f, float spread = 1.f,
    //                       uint16_t resolution = 128)
    // {
    //     running.push<Effects::DimmerChase>(curve, bpm, spread, resolution);
    // }

    // ---- clear (staged, console-style) ----
    void clearCurrent() {}
    // void clearValues() { m_edits.clear(); } // keep selection
    void clearAll()
    {
        // running.stacks.clear();
        // running.createNew(); // keep an active stack so c_ptr stays valid
    } // wipe both

    // int priority() const override { return 1000; } // programmer wins
    void apply(Frame &frame, const TimeContext &time) override;

    // A copy of the live selection - Engine wraps this into a Pools::Group on
    // store. Keeps the programmer free of any pool-object dependency.
    // [[nodiscard]] DMX::FixtureGroup selectedGroup() const
    // {
    //     return m_selection;
    // }
};

// class ProgrammerLayer : public Layer
// {
//     // The live selection: transient, ordered, cached. NOT a Pools::Group -
//     that
//     // is a stored object; this is "what I'm editing right now". Engine
//     // snapshots it into a Pools::Group on storeGroup().
//     Patch &m_patch; // resolves raw FIDs -> live fixtures for selection
//     DMX::FixtureGroup m_selection;
//     // Everything the programmer produces is an effect. Static edits (an 'at'
//     // level, a colour) are just constant effects; each edit pushes a new one
//     and
//     // the latest LTP-wins, so the stack reads back as the current live
//     state. std::vector<std::unique_ptr<Effects::Effect>> m_effects;

//     // A group holding a single patched fixture, for per-fixture effects.
//     [[nodiscard]] DMX::FixtureGroup singleFixture(uint16_t fid) const;
//     void push(std::unique_ptr<Effects::Effect> e)
//     {
//         m_effects.push_back(std::move(e));
//     }

// public:
//     explicit ProgrammerLayer(Patch &patch) : m_patch(patch) {}

//     // ---- selection ----
//     // select() replaces the current selection, add() accumulates onto it.
//     // FixtureGroup overloads take an already-resolved selection; the FID
//     // overloads resolve through the patch (skips unpatched FIDs).
//     void select(const DMX::FixtureGroup &g)
//     {
//         m_selection.clear();
//         m_selection.add(g);
//     }
//     void select(const std::vector<uint16_t> &fids)
//     {
//         m_selection.clear();
//         m_selection.add(m_patch.getFixtures(fids));
//     }
//     void add(const DMX::FixtureGroup &g) { m_selection += g; }
//     void add(const std::vector<uint16_t> &fids)
//     {
//         m_selection.add(m_patch.getFixtures(fids));
//     }
//     void deselect() { m_selection.clear(); }
//     [[nodiscard]] const DMX::FixtureGroup &selection() const
//     {
//         return m_selection;
//     }

//     // ---- edits (scoped to the current selection) ----
//     void setColor(const Utils::Colors::HSV &hsv); // sets hue/sat only
//     void setColor(const Utils::Colors::RGB &rgb); // converts to HSV, hue/sat
//     void setHueSat(float h, float s);
//     void setIntensity(float v);
//     // Distribute intensity a..b across the selection in order (t = i/(n-1)).
//     void setIntensityRamp(float a, float b);
//     // Fan hue hueA..hueB across the selection in order, at fixed saturation.
//     void fanColor(float hueA, float hueB, float sat = 1.f);

//     // Per-fixture setters used by preset recall (bypass the selection): each
//     // pushes a single-fixture constant effect.
//     void applyHueSat(uint16_t fid, float h, float s);
//     void applyIntensity(uint16_t fid, float v);

//     // Flatten the effect stack into per-fixture values (constant effects
//     sampled
//     // at t=0). This is the programmer's current live state; preset store
//     reads
//     // it, so it keeps working unchanged.
//     [[nodiscard]] std::map<uint16_t, FixtureValues> edits() const;

//     // ---- clear (staged, console-style) ----
//     void clearValues() { m_effects.clear(); } // keep selection
//     void clearAll()
//     {
//         m_selection.clear();
//         m_effects.clear();
//     } // wipe both

//     int priority() const override { return 1000; } // programmer wins
//     void apply(Frame &frame, const TimeContext &time) override;

//     // A copy of the live selection - Engine wraps this into a Pools::Group
//     on
//     // store. Keeps the programmer free of any pool-object dependency.
//     [[nodiscard]] DMX::FixtureGroup selectedGroup() const { return
//     m_selection; }
// };
} // namespace LightEngine::Engine
