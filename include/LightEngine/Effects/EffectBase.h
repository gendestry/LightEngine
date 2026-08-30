#pragma once

#include <cstdint>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Engine/Layers/Frame.h"
#include "Utils/Math/Curve.h"
#include "Utils/Time/TimeContext.h"

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

#define EFFECT_TYPE(type)                                          \
    static EffectType GetStaticType() { return EffectType::type; } \
    virtual EffectType GetEffectType() const override              \
    {                                                               \
        return GetStaticType();                                    \
    }                                                               \
    virtual const char *GetTypeName() const override { return #type; }

#define EFFECT_CATEGORY(type)                                                  \
    static EffectCategory GetStaticCategory() { return EffectCategory::type; } \
    virtual EffectCategory GetEffectCategory() const override                  \
    {                                                                          \
        return GetStaticCategory();                                            \
    }                                                                          \
    virtual const char *GetCategoryName() const override { return #type; }

// Deep-copies via T's copy constructor, then marks the copy dirty - the
// inherited m_cache belongs to the source's group, not the clone's.
#define EFFECT_CLONE(T)                                        \
    virtual std::shared_ptr<EffectBase> clone() const override \
    {                                                           \
        auto c = std::make_shared<T>(*this);                    \
        c->markDirty();                                          \
        return c;                                                \
    }

class EffectBase
{
public:
    // Returned by stepInterval() to mean "never wakes on its own".
    static constexpr double NEVER = std::numeric_limits<double>::infinity();

protected:
    bool m_enabled = true;

    // The last computed output, replayed on frames where this effect is not
    // due. Rebuilt wholesale by recompute() - effects are pure functions of
    // (params, time, selection), so there is nothing to merge incrementally.
    std::vector<std::pair<uint16_t, Engine::FixtureValues>> m_cache;
    Engine::MergePolicy m_policy = Engine::MergePolicy::LTP;

    bool m_dirty = true;
    uint64_t m_cachedRevision = 0;

    double m_nextDue = 0.0;
    double m_phaseOrigin = 0.0;

    virtual void recompute(const Utils::Time::TimeContext &t, const Utils::Maths::Interval &group) = 0;
    void emit(uint16_t fid, const Engine::FixtureValues &values);

public:
    virtual ~EffectBase() = default;

    [[nodiscard]] virtual double stepInterval() const { return NEVER; }
    [[nodiscard]] bool due(double now, const Utils::Maths::Interval &group) const;

    // Recompute the cache and schedule the next wake-up.
    void evaluate(const Utils::Time::TimeContext &t, const Utils::Maths::Interval &group);

    // Merge the cached output into this frame. Cheap: no effect logic runs.
    void replay(Engine::Frame &frame) const;

    // Any parameter change must call this, or the edit never reaches the cache.
    void markDirty() { m_dirty = true; }

    // Advance m_nextDue onto the next point of this effect's own grid. Anchored
    // to m_phaseOrigin rather than `now` on purpose: `now + interval` would
    // accumulate the frame's timing jitter and walk the effect off the beat.
    void scheduleNext(double now);
    void syncPhase(double now); // Re-anchor the beat grid to `now` (effect start, BPM change, tap-sync).

    [[nodiscard]] double nextDue() const { return m_nextDue; }

    void setEnabled(bool enabled) { m_enabled = enabled; }
    [[nodiscard]] bool enabled() const { return m_enabled; }

    [[nodiscard]] const std::vector<std::pair<uint16_t, Engine::FixtureValues>> &getValues() const { return m_cache; }

    // Deep copy, used by store/recall so a stored effect never aliases the
    // live one it was cloned from.
    [[nodiscard]] virtual std::shared_ptr<EffectBase> clone() const = 0;

    virtual EffectType GetEffectType() const = 0;
    virtual const char *GetTypeName() const = 0;

    virtual EffectCategory GetEffectCategory() const = 0;
    virtual const char *GetCategoryName() const = 0;
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
    EffectAnimated() = default;

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
