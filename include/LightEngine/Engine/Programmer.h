#pragma once
#include <cstdint>
#include <map>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Engine/Frame.h"
#include "LightEngine/Engine/Layer.h"
#include "LightEngine/Engine/Patch.h"
#include "LightEngine/Engine/TimeContext.h"

//
// Layer: anything that produces values for a frame. Programmer, playback and
// effects are all just layers. update() composes them low priority -> high, so
// a higher-priority layer's LTP writes land on top of a lower one's.
//
namespace LightEngine::Engine
{

// The live editing layer - what the CommandBuilder/CLI drives. Holds the user's
// edits per fixture; a selection scopes the bulk setters.
class Programmer : public Layer
{
    // The live selection: transient, ordered, cached. NOT a Pools::Group - that
    // is a stored object; this is "what I'm editing right now". Engine
    // snapshots it into a Pools::Group on storeGroup().
    Patch &m_patch; // resolves raw FIDs -> live fixtures for selection
    DMX::FixtureGroup m_selection;
    std::map<uint16_t, FixtureValues> m_edits; // FID -> touched values

    Utils::Colors::HSV &ensureColor(uint16_t fid);

public:
    explicit Programmer(Patch &patch) : m_patch(patch) {}

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

    // ---- edits (scoped to the current selection) ----
    void setColor(const Utils::Colors::HSV &hsv); // sets hue/sat only
    void setColor(const Utils::Colors::RGB &rgb); // converts to HSV, hue/sat
    void setHueSat(float h, float s);
    void setIntensity(float v);
    // Distribute intensity a..b across the selection in order (t = i/(n-1)).
    void setIntensityRamp(float a, float b);
    // Fan hue hueA..hueB across the selection in order, at fixed saturation.
    void fanColor(float hueA, float hueB, float sat = 1.f);

    // Per-fixture setters used by preset recall (bypass the selection).
    void applyHueSat(uint16_t fid, float h, float s)
    {
        Utils::Colors::HSV &c = ensureColor(fid);
        c.h = h;
        c.s = s;
    }
    void applyIntensity(uint16_t fid, float v) { m_edits[fid].intensity = v; }
    [[nodiscard]] const std::map<uint16_t, FixtureValues> &edits() const
    {
        return m_edits;
    }

    // ---- clear (staged, console-style) ----
    void clearValues() { m_edits.clear(); } // keep selection
    void clearAll()
    {
        m_selection.clear();
        m_edits.clear();
    } // wipe both

    int priority() const override { return 1000; } // programmer wins
    void apply(Frame &frame, const TimeContext &time) override;

    // A copy of the live selection - Engine wraps this into a Pools::Group on
    // store. Keeps the programmer free of any pool-object dependency.
    [[nodiscard]] DMX::FixtureGroup selectedGroup() const
    {
        return m_selection;
    }
};
} // namespace LightEngine::Engine
