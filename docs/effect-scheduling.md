# Effect Scheduling — evaluate on demand, not every frame

Status: implemented (steps 1–2 of 7; the engine-level short-circuit is still to
come — see [Remaining work](#remaining-work))

## Problem

The render loop ran every effect, in full, on every frame:

```cpp
// Programmer::apply — before
for (const auto &stack : running.stacks)
    for (const auto &e : stack.effects)
        if (e->enabled())
            e->apply(frame, time, stack.selection);
```

At 40 fps, a "select 500 fixtures, set them red at full" show did this 40 times a
second, forever:

- `StaticColor::apply` converted the same RGB to HSV, then walked 500 fixtures
  calling `frame.contribute()` — for a value that had not changed since the user
  typed it.
- `FixtureGroup::fids()` built and returned a **fresh `std::vector<uint16_t>`**
  on every call — one heap allocation per effect per frame.

The cost scaled with `fixtures × effects × frame rate`, while the *information*
being produced changed only when the operator touched something. Static looks —
which is most of what a console holds at any moment — paid the full animated
price.

The naive fix is a dirty flag: recompute when edited, otherwise reuse. That
works for static values and falls apart the moment effects animate, because each
effect has **its own rate**. A 128-step chase at 120 BPM has new output 256
times a second; a 4-step chase at 60 BPM has new output 4 times a second; a
static colour never does. A single global "something changed" flag cannot tell
you *which* of those needs attention on this particular frame.

## The model

Two independent questions, asked per effect, per frame:

1. **Has someone edited me?** → a dirty flag. Covers parameter changes and
   selection changes.
2. **Has my own clock ticked?** → a *due time*. Covers animation.

Either one makes the effect **due**. A due effect recomputes; a non-due effect
replays what it computed last time. The key property is that question 2 is
per-effect, so effects at different BPMs coexist without a shared tick and
without anyone re-running effects that had nothing new to say.

### recompute / replay

`EffectBase::apply()` is gone, split into two halves with very different costs:

```cpp
// expensive: colour maths, curve lookups, walking the selection.
// Subclass hook. Called only on frames where the effect is due.
virtual void recompute(const TimeContext &t, const FixtureGroup &group) = 0;

// cheap: a merge. Called every frame.
void replay(Frame &frame) const;
```

`recompute()` fills a private cache of `(fid, FixtureValues)` pairs via
`emit()`; `replay()` walks that cache into the frame via `frame.contribute()`.

```cpp
std::vector<std::pair<uint16_t, Engine::FixtureValues>> m_cache;
Engine::MergePolicy m_policy = Engine::MergePolicy::LTP;
```

The cache is rebuilt wholesale, never patched incrementally — an effect is a
pure function of `(parameters, time, selection)`, so there is nothing to merge.
`evaluate()` is the wrapper that clears the cache, calls `recompute()`, records
what the cache was built from, and schedules the next wake-up.

Concrete effects only implement `recompute()`. `StaticColor` shows what moves
out of the hot path:

```cpp
void StaticColor::recompute(const TimeContext &, const FixtureGroup &group)
{
    FixtureValues v;
    v.color = Utils::Colors::rgbToHsv(m_color); // once per edit, not per frame

    m_cache.reserve(group.size());
    for (uint16_t fid : group.fids())
        emit(fid, v);
}
```

### Invalidation: edits

```cpp
bool m_dirty = true;          // starts dirty -> first frame always computes
uint64_t m_cachedRevision = 0;
```

Every parameter setter must call `markDirty()`. This is not optional
bookkeeping — with a cache in place, a setter that mutates silently produces an
edit that never reaches the stage. `StaticColor::setColor` and
`StaticIntensity::setLevel` were both updated accordingly.

Selection changes are caught differently. An effect does not own its
`FixtureGroup` (the stack does), so it cannot be notified when the group is
edited. Instead `FixtureGroup` carries a monotonic **revision counter**, bumped
on every membership change, and the effect stores the revision its cache was
built from:

```cpp
bool due(double now, const FixtureGroup &group) const
{
    return m_dirty || group.revision() != m_cachedRevision || now >= m_nextDue;
}
```

That is the whole per-frame check for a static effect: two comparisons and an
integer compare, in place of a full re-evaluation.

Revision is used here rather than `fidsHash()` deliberately. The hash is O(n log
n) — right for *identity* questions (are these two groups the same selection?
key a preset by its target), wrong for a check that runs every frame per effect.
`revision()` answers "did this group change?" in O(1). The two coexist:

| | `revision()` | `fidsHash()` |
|---|---|---|
| Cost | O(1) | O(n log n) |
| Answers | did *this* group change | are two groups the *same* selection |
| Order-sensitive | n/a | no (sorted before hashing) |
| Use for | per-frame cache validity | dedup, keying, comparison |

Because a consumer's validity test is `cached == revision()`, the counter must
never move backwards or be reused. Copy-assignment was the hole: the implicit
`operator=` copied the source's revision, so `groupA = groupB` could hand a
consumer a number it had already seen and validate a stale cache. `FixtureGroup`
now defines assignment by hand to *bump* the counter instead of copying it, and
to mark the derived caches dirty (the cached `Parameter*`/`ColorCell*` pointers
described the old membership).

### Invalidation: time

Each effect declares its own cadence:

```cpp
// seconds between steps that actually produce different output
virtual double stepInterval() const { return NEVER; }  // = infinity
```

Three regimes:

| `stepInterval()` | Meaning | Recomputes |
|---|---|---|
| `NEVER` (∞) | static, or a stopped effect | only on an edit |
| `0` | continuously varying (e.g. a sine fade) | every frame |
| `> 0` | stepped animation | on its own step boundaries |

`EffectStatic` inherits the `NEVER` default and needs no code at all.
`EffectAnimated` derives its interval from BPM:

```cpp
double stepInterval() const override
{
    if (m_bpm <= 0.f) return NEVER;          // stopped: costs nothing
    const double beat = 60.0 / m_bpm;
    return m_steps > 0 ? beat / m_steps : 0.0;
}
```

`m_steps` is steps per beat. At 120 BPM with 4 steps per beat that is 8
recomputes a second instead of 40 — and crucially, a *continuous* effect
returning `0` is still due every frame, but it is then the only thing paying
that cost rather than dragging every other effect along with it.

### Scheduling on the beat grid

The one subtle piece. After evaluating, the next due time is **not**
`now + interval`:

```cpp
void scheduleNext(double now)
{
    const double interval = stepInterval();
    if (!std::isfinite(interval)) { m_nextDue = NEVER; return; }  // static
    if (interval <= 0.0)          { m_nextDue = now;   return; }  // continuous

    const double steps = std::floor((now - m_phaseOrigin) / interval) + 1.0;
    m_nextDue = m_phaseOrigin + steps * interval;
}
```

`now + interval` would compound the render loop's timing jitter: each frame
arrives a little late, each step is scheduled from that late arrival, and the
chase gradually walks off the beat. Anchoring to an absolute grid
(`m_phaseOrigin + n × interval`) means a late frame catches up on the next step
instead of pushing the whole sequence back. Errors do not accumulate.

`m_phaseOrigin` is the anchor: effect start, or the last BPM change / tap-sync.
`syncPhase(now)` re-anchors it and marks the effect dirty; `setBpm(bpm, now)`
calls it so a rate change does not produce a visible jump mid-cycle.

### The loop

```cpp
// Programmer::apply — after
for (auto &stack : running.stacks)
{
    const auto &group = stack.selection;
    for (const auto &e : stack.effects)
    {
        if (!e->enabled()) continue;
        if (e->due(time.now, group))
            e->evaluate(time, group);   // fills the cache, reschedules
        e->replay(frame);               // merges the cache into the frame
    }
}
```

Note what is *not* here: no asking "which effect changed", no central schedule,
no sorting by due time. Each effect answers for itself in one branch. Adding an
effect type with a new cadence requires overriding `stepInterval()` and nothing
else.

## Supporting change: `FixtureGroup::fids()`

`fids()` built a fresh vector on every call, which the effects hit once per
frame each. It is now folded into the group's existing lazy cache — filled in
`rebuildCache()` alongside `m_byAttribute`/`m_colorCells`, and returned by
reference:

```cpp
[[nodiscard]] const std::vector<uint16_t> &fids() const;
```

Range-for call sites needed no change. `Pools::Group::fids()` still returns a
copy, which is what it wants for serialization, and `fidsHash()` takes an
explicit copy before sorting.

## What this buys

For a static look — the common case — per frame, per effect:

| | Before | After |
|---|---|---|
| Heap allocations | 1 (`fids()`) | 0 |
| RGB→HSV conversions | 1 | 0 |
| `frame.contribute()` calls | n | n |
| Effect logic | full | one branch |

For a 120 BPM / 4-step chase at 40 fps: 8 recomputes a second instead of 40, on
the beat grid rather than the frame grid. For a stopped effect: nothing.

`contribute()` still runs per fixture per frame, because the frame is rebuilt
from scratch each tick — that is what the remaining work addresses.

## Remaining work

The per-effect saving is in place; the structural saving is not. `Engine::update`
still blackouts every universe, composes the frame and resolves every fixture on
every frame, so the floor cost is unchanged.

Since every effect can now report due-ness, the engine can determine that a
frame is a **no-op**: if no effect was due and no stack is dirty, the composed
frame is bit-identical to the last one, and `blackout()` + compose + `Resolve()`
can all be skipped in favour of re-sending the existing universe buffers. An
idle show then costs approximately nothing, and a single chase costs only its own
recomputes.

Planned, in order:

1. **Per-stack compose cache** — `EffectStack` holds its composed `Frame`,
   invalidated when any of its effects recomputes or its selection revision
   moves. Also fixes an existing bug: `EffectHolder::push()` never sets
   `dirty`, so `select()` can never open a new stack and the stack model is
   currently inert.
2. **`Programmer::apply` returns whether anything changed**, so the engine can
   short-circuit.
3. **`Engine::update`** — hoist the per-frame `std::stable_sort` of layers into
   `addLayer()`, add the no-op short-circuit, and merge-walk `frame.all()`
   against `patch.fixtures()` (both sorted by fid) instead of a `std::map`
   lookup per fixture.
4. **`Frame::clear()`** — reset values in place rather than destroying every
   `std::map` node each frame and reallocating them the next.
5. **Effect creation stamps `syncPhase()`** — `m_phaseOrigin` is currently 0,
   which is correct only because the engine clock starts at 0. Effects created
   mid-show need anchoring to the real `now`.

An additional payoff once due-ness is engine-visible: `min(nextDue)` across all
effects is the engine's next required wake-up, which would let the render loop
be driven by a timer instead of a fixed 25 ms sleep.

## Files

| File | Change |
|---|---|
| `include/LightEngine/Effect/EffectBase.h` | cache + schedule state, `recompute`/`replay` split, `due`/`evaluate`/`scheduleNext`/`syncPhase`; `EffectAnimated::stepInterval` from BPM |
| `include/LightEngine/DMX/FixtureGroup.h`, `src/DMX/FixtureGroup.cpp` | cached `fids()`, `revision()`, `fidsHash()`, hand-written assignment |
| `include/LightEngine/Effect/Static/EffectColor.{h}`, `src/Effect/Static/EffectColor.cpp` | `apply` → `recompute`; `setColor` marks dirty |
| `include/LightEngine/Effect/Static/EffectIntensity.h`, `src/Effect/Static/EffectIntensity.cpp` | `apply` → `recompute`; `setLevel` marks dirty |
| `include/LightEngine/Effect/Animated/EffectDimmerChase.h` | declaration ported (no implementation yet) |
| `src/Layers/Programmer.cpp` | the due-check loop |

## Related

- [`effect-stack-design.md`](effect-stack-design.md) — the stack structure this
  caching hangs off
- [`ENGINE.md`](ENGINE.md) — the frame/layer/resolve pipeline overall
