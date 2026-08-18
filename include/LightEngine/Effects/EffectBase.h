#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "LightEngine/DMX/FixtureGroup.h"
// #include "LightEngine/Effects/EffectSpec.h"
#include "LightEngine/Engine/Layers/Frame.h"
#include "LightEngine/Engine/TimeContext.h"
#include "Utils/Math/Curve.h"

//
// Effect: a unit of behaviour, evaluated only when it has something new to say.
//
// An effect does NOT write into the frame directly. It recompute()s its output
// into a private cache of (fid, FixtureValues) pairs, and replay()s that cache
// into the frame every frame. Recomputing is the expensive part (colour maths,
// curve lookups, walking the selection); replaying is a merge.
//
// Two independent things make an effect due for recompute:
//   - m_dirty       - a parameter or the selection changed (a user edit)
//   - m_nextDue     - its own animation grid says a new step has arrived
//
// A static effect's stepInterval() is infinite, so it recomputes once and then
// only ever on an edit. An effect at 120 BPM with 4 steps per beat recomputes 8
// times a second regardless of frame rate. A continuous effect returns 0 and is
// due every frame - but it is then the only thing paying that cost.
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

#define EFFECT_TYPE(type)                                                      \
    static EffectType GetStaticType() { return EffectType::type; }             \
    virtual EffectType GetEffectType() const override                          \
    {                                                                          \
        return GetStaticType();                                                \
    }                                                                          \
    virtual const char *GetTypeName() const override { return #type; }

#define EFFECT_CATEGORY(type)                                                  \
    static EffectCategory GetStaticCategory() { return EffectCategory::type; } \
    virtual EffectCategory GetEffectCategory() const override                  \
    {                                                                          \
        return GetStaticCategory();                                            \
    }                                                                          \
    virtual const char *GetCategoryName() const override { return #type; }

class EffectBase
{
public:
    // Returned by stepInterval() to mean "never wakes on its own".
    static constexpr double NEVER = std::numeric_limits<double>::infinity();

protected:
    bool m_enabled = true;
    // EffectType m_type;

    // The last computed output, replayed on frames where this effect is not
    // due. Rebuilt wholesale by recompute() - effects are pure functions of
    // (params, time, selection), so there is nothing to merge incrementally.
    std::vector<std::pair<uint16_t, Engine::FixtureValues>> m_cache;
    Engine::MergePolicy m_policy = Engine::MergePolicy::LTP;

    // Invalidation: an edit sets m_dirty; a selection change is caught by
    // comparing the group's revision against the one the cache was built from.
    bool m_dirty = true;
    uint64_t m_cachedRevision = 0;

    // Scheduling. m_phaseOrigin is the anchor of this effect's own beat grid
    // (start time, or the last BPM change / tap-sync).
    double m_nextDue = 0.0;
    double m_phaseOrigin = 0.0;

    // Subclass hook: fill m_cache (already cleared) via emit(). Called only on
    // frames where this effect is due.
    virtual void recompute(const Engine::TimeContext &t,
                           const DMX::FixtureGroup &group) = 0;

    void emit(uint16_t fid, const Engine::FixtureValues &values)
    {
        m_cache.emplace_back(fid, values);
    }

public:
    // EffectBase(EffectType type) : m_type(type) {}
    virtual ~EffectBase() = default;

    // Seconds between steps that actually produce different output. NEVER for
    // a static effect (the default), 0 for a continuously varying one.
    [[nodiscard]] virtual double stepInterval() const { return NEVER; }

    // Does this effect need recomputing this frame? One comparison in the
    // common case - this is what replaces re-running the effect every frame.
    [[nodiscard]] bool due(double now, const DMX::FixtureGroup &group) const
    {
        return m_dirty || group.revision() != m_cachedRevision ||
               now >= m_nextDue;
    }

    // Recompute the cache and schedule the next wake-up.
    void evaluate(const Engine::TimeContext &t, const DMX::FixtureGroup &group)
    {
        m_cache.clear();
        recompute(t, group);
        m_cachedRevision = group.revision();
        m_dirty = false;
        scheduleNext(t.now);
    }

    // Merge the cached output into this frame. Cheap: no effect logic runs.
    void replay(Engine::Frame &frame) const
    {
        for (const auto &[fid, values] : m_cache)
        {
            frame.contribute(fid, values, m_policy);
        }
    }

    // Any parameter change must call this, or the edit never reaches the cache.
    void markDirty() { m_dirty = true; }

    // Advance m_nextDue onto the next point of this effect's own grid. Anchored
    // to m_phaseOrigin rather than `now` on purpose: `now + interval` would
    // accumulate the frame's timing jitter and walk the effect off the beat.
    void scheduleNext(double now)
    {
        const double interval = stepInterval();
        if (!std::isfinite(interval))
        {
            m_nextDue = NEVER; // static: only an edit brings it back
            return;
        }
        if (interval <= 0.0)
        {
            m_nextDue = now; // continuous: due again immediately
            return;
        }
        const double steps = std::floor((now - m_phaseOrigin) / interval) + 1.0;
        m_nextDue = m_phaseOrigin + steps * interval;
    }

    // Re-anchor the beat grid to `now` (effect start, BPM change, tap-sync).
    void syncPhase(double now)
    {
        m_phaseOrigin = now;
        m_dirty = true;
    }

    [[nodiscard]] double nextDue() const { return m_nextDue; }

    // EffectType getType() const { return m_type; }
    // [[nodiscard]] virtual Spec spec() const = 0;

    void setEnabled(bool enabled) { m_enabled = enabled; }
    [[nodiscard]] bool enabled() const { return m_enabled; }

    virtual EffectType GetEffectType() const = 0;
    virtual const char *GetTypeName() const = 0;

    virtual EffectCategory GetEffectCategory() const = 0;
    virtual const char *GetCategoryName() const = 0;

    // virtual std::string describe() const = 0;
};

class EffectStatic : public EffectBase
{
public:
    // EffectStatic() : EffectBase(EffectType::STATIC) {}
    EffectStatic() : EffectBase() {}

    EFFECT_TYPE(STATIC);
};

class EffectAnimated : public EffectBase
{
protected:
    std::unique_ptr<Utils::Maths::Curve> m_curve;
    float m_bpm = 60.f;
    float m_spread = 1.f;
    // Steps per beat. 0 means "sample the curve continuously" -> due every
    // frame; anything else quantises the effect onto that many steps per beat.
    uint16_t m_steps = 0;

public:
    // EffectAnimated() : EffectBase(EffectType::ANIMATED) {}
    EffectAnimated() : EffectBase() {}

    EFFECT_TYPE(ANIMATED);

    // One beat is 60/bpm seconds; a step is that divided by m_steps. A stopped
    // effect (bpm <= 0) is frozen and only wakes on an edit.
    [[nodiscard]] double stepInterval() const override
    {
        if (m_bpm <= 0.f)
        {
            return NEVER;
        }
        const double beat = 60.0 / static_cast<double>(m_bpm);
        return m_steps > 0 ? beat / static_cast<double>(m_steps) : 0.0;
    }

    // Changing the rate re-anchors the grid to avoid a jump mid-cycle; the
    // caller passes the current engine time.
    void setBpm(float bpm, double now)
    {
        m_bpm = bpm;
        syncPhase(now);
    }
    [[nodiscard]] float bpm() const { return m_bpm; }
    void setSpread(float spread)
    {
        m_spread = spread;
        markDirty();
    }
    [[nodiscard]] float spread() const { return m_spread; }
    void setSteps(uint16_t steps)
    {
        m_steps = steps;
        markDirty();
    }
    [[nodiscard]] uint16_t steps() const { return m_steps; }
};
} // namespace LightEngine::Effects
