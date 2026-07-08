#pragma once
#include <cstdint>
#include <map>
#include <vector>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Engine/Frame.h"
#include "LightEngine/Engine/TimeContext.h"

//
// Layer: anything that produces values for a frame. Programmer, playback and
// effects are all just layers. update() composes them low priority -> high, so
// a higher-priority layer's LTP writes land on top of a lower one's.
//
namespace LightEngine::Engine
{
class Layer
{
public:
    virtual ~Layer() = default;

    [[nodiscard]] virtual int priority() const = 0; // sort key, low -> high
    [[nodiscard]] virtual bool enabled() const { return true; }

    // Write this layer's contribution into the frame for this tick.
    virtual void apply(Frame &frame, const TimeContext &time) = 0;
};

// The live editing layer - what the CommandBuilder/CLI drives. Holds the user's
// edits per fixture; a selection scopes the bulk setters.
class ProgrammerLayer : public Layer
{
    std::vector<uint16_t> m_selection;          // FIDs the setters apply to
    std::map<uint16_t, FixtureValues> m_edits;  // FID -> user-set values

    Utils::Colors::HSV &ensureColor(uint16_t fid);

public:
    // ---- selection ----
    void select(const DMX::FixtureGroup &group);
    void select(std::vector<uint16_t> fids) { m_selection = std::move(fids); }
    void deselect() { m_selection.clear(); }

    // ---- edits (applied to the current selection) ----
    void setColor(const Utils::Colors::HSV &hsv); // sets hue/sat/intensity
    void setHueSat(float h, float s);
    void setIntensity(float v);
    // Distribute intensity a..b across the selection in order (t = i/(n-1)).
    void setIntensityRamp(float a, float b);

    void clear() { m_edits.clear(); } // wipe all edits (keeps selection)

    int priority() const override { return 1000; } // programmer wins
    void apply(Frame &frame, const TimeContext &time) override;
};
} // namespace LightEngine::Engine
