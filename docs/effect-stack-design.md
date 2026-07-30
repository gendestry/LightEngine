# Grouped Effect Stacks — Programmer design note

Status: proposal (design-first, no code yet)

## Problem

Today `Programmer` owns a **flat** list:

```cpp
std::vector<std::unique_ptr<Effects::Effect>> m_effects;
```

Every `set*` command pushes one effect; `apply()` replays them low→high each
frame and LTP-wins gives the live state. This works, but the structure is lost:
there is no record of *which selection an edit was made under*, so a stored
programmer is just an undifferentiated pile of effects.

## Proposal

Group the flat list into **effect-stacks**. Each stack is a
`(selectionSnapshot, effects[])` block. Editing commands (`set …`) append to the
*current* stack; a **selection change opens a new stack**.

```
group1 = 1-10
group2 = 11-30

select group1        → open stack #1, selection = {1-10}
set dimmer 100         → stack#1 += StaticIntensity
set color 25           → stack#1 += StaticColor
addselect group2     → open stack #2, selection = {1-10, 11-30}
set dimmerchase …      → stack#2 += DimmerChase
```

Resulting model:

```
Programmer
  stack#1  sel={1-10}          [ StaticIntensity(100), StaticColor(25) ]
  stack#2  sel={1-10,11-30}    [ DimmerChase(…) ]
```

### Stack boundary rule (decided)

A **new stack opens on *any* selection change** — both `select` (replace) and
`addselect` (accumulate). The new stack's `selectionSnapshot` is the selection
state *after* the change, i.e. `addselect group2` snapshots `{group1 ∪ group2}`.

Consequence: consecutive `set` commands with no intervening selection change all
land in the same stack. A `set` issued when there is **no** current stack (e.g.
before any `select`) is an error / no-op — surface it, don't silently create a
stack with an empty selection.

## Data model

```cpp
namespace LightEngine::Engine {

struct EffectStack {
    DMX::FixtureGroup selection;                              // snapshot at open time
    std::vector<std::unique_ptr<Effects::Effect>> effects;    // in command order
};

class Programmer : public Layer {
    Patch& m_patch;
    DMX::FixtureGroup m_selection;          // still the *live* selection
    std::vector<EffectStack> m_stacks;      // replaces flat m_effects
    // ...
};
```

`m_selection` stays as the live selection driving new pushes; `m_stacks` is the
recorded structure. Each `Effect` already owns its target `FixtureGroup` by
value, so `EffectStack::selection` is partly redundant with the effects' own
groups — but it's the authoritative record of the block's intent and is what a
per-fixture effect (fan/ramp) is derived from, so it's worth keeping explicit.

## Command → stack mapping

| Command                | Effect on `m_stacks`                                            |
|------------------------|-----------------------------------------------------------------|
| `select g`             | `m_selection = g`; **push new** `EffectStack{selection=g}`      |
| `addselect g`          | `m_selection += g`; **push new** `EffectStack{selection=m_selection}` |
| `deselect`             | `m_selection.clear()`; close current stack (no new one opened)  |
| `set dimmer/color/…`   | append effect to `m_stacks.back()` (error if none open)         |
| `clearAll`             | `m_selection.clear(); m_stacks.clear()`                         |

Helper:

```cpp
void openStack() { m_stacks.push_back(EffectStack{m_selection, {}}); }
void push(std::unique_ptr<Effects::Effect> e) {
    assert(!m_stacks.empty());      // set with no active selection = error
    m_stacks.back().effects.push_back(std::move(e));
}
```

## apply()

Unchanged semantics — flatten stacks in order, low→high, LTP-wins:

```cpp
void Programmer::apply(Frame& frame, const TimeContext& t) {
    for (auto& stack : m_stacks)
        for (auto& e : stack.effects)
            if (e->enabled()) e->apply(frame, t);
}
```

Order across stacks is command order, so later stacks still win over earlier
ones for overlapping fixtures — same as today's flat list.

## Store / recall

A stored programmer now serializes as a list of blocks, each with its FID
snapshot + the effects' `Spec`s. Recall rebuilds `EffectStack`s via
`EffectFactory`. This is strictly richer than the flat model and needs the
selection snapshot that this design adds.

## Open questions (not blocking)

1. **Same-parameter overwrite within a stack** — `set dimmer 100` then
   `set dimmer 50` in one stack pushes two `StaticIntensity`; LTP makes 50 win.
   Fine functionally, but the stack grows. Do we want to *replace* the prior
   same-kind/same-target effect instead of appending? (Recommend: append for
   now; dedupe later if the stack view gets noisy.)
2. **Empty stacks** — a `select` immediately followed by another `select` leaves
   an empty stack. Prune on the next `select`, or leave and let the UI hide it?
   (Recommend: prune the current stack on selection change if it has no effects.)
3. **Enable/disable granularity** — do we want to toggle a whole stack, not just
   individual effects? Cheap to add an `EffectStack::enabled` flag.

---

# Layers, playbacks and the merge model (executor design)

Goal: a ChamSys exec / MA sequence — a playback you can place relative to the
programmer (above / below) and configure HTP vs LTP per attribute.

## Why the current model can't express it

All layers write into one shared `m_frame`, and each **effect** hard-codes its
`MergePolicy` at `contribute()` time. So there is no per-playback "sit above the
programmer" or "HTP/LTP" switch — the effect already decided. The merge decision
must move **off the effect and onto the layer**.

## Two-pass composition (decided)

1. **Each layer renders to its own buffer.** `Layer::render(TimeContext) -> Frame`
   replaces `apply(Frame&, time)`. Within a layer, effect-stacks/effects compose
   LTP-by-order into that private frame (latest edit wins — right for both the
   programmer and a single cue).
2. **Engine folds layers by priority into the master frame**, using each layer's
   `MergeConfig`.

```cpp
struct MergeConfig {
    Layer::Priority priority;            // LOW..ABOVE_PROG — placement
    MergePolicy intensity = MergePolicy::HTP;   // the ChamSys HTP/LTP toggle
    MergePolicy color     = MergePolicy::LTP;
    MergePolicy generic   = MergePolicy::LTP;
};

class Layer {
    virtual Frame render(const TimeContext&) = 0;
    MergeConfig merge;
};
```

## Fold rule — MA-style, priority beats HTP (decided)

Per attribute, per fixture:

- **LTP attributes** (color/generic): highest-priority layer holding a value
  wins outright.
- **HTP attributes** (intensity): find the **highest priority tier** with any
  dimmer value; **max within that tier**; ignore lower tiers.

Behaviors this yields:

| Action | Result |
|---|---|
| Playback parked `ABOVE_PROG` | its dimmer/color overrides the programmer |
| Two playbacks same tier | HTP max on dimmer, latest-loaded wins color |
| Playback set intensity=LTP | overrides same-tier HTP instead of maxing |
| Programmer at `PROG` | wins while editing unless a playback is `ABOVE_PROG` |

## Playback layer

```cpp
class Playback : public Layer {
    std::vector<EffectStack> m_cues;   // same brick as the programmer
    size_t m_active = 0;
    // MergeConfig (inherited) is the exec's option page:
    //   priority, per-attr HTP/LTP, (later) release behaviour, fade times…
};
```

Final shape:

```
Engine — folds layers low→high by MergeConfig.priority, per-attr HTP/LTP
 ├─ Playback   (ABOVE_PROG, dimmer=HTP)   ChamSys exec / MA sequence
 ├─ Programmer (PROG, all LTP)
 └─ Playback   (NORMAL, dimmer=HTP)
      └─ EffectStack (cue) → Effect…      same brick everywhere
```

## Migration impact

- `Layer`: swap `apply(Frame&, time)` → `render(time) -> Frame`; add `MergeConfig`.
- `Programmer`: `render()` builds its private frame from `m_stacks` (all LTP);
  drop per-effect policy reliance.
- `Effect::apply(Frame&, t)`: still writes into the (now layer-private) frame,
  but the **layer**, not the effect, owns the cross-layer policy. Effects can
  keep contributing their attributes; the LTP-by-order within a layer is
  unchanged.
- `Engine::update()`: collect `render()` from each layer, fold by priority into
  `m_frame` using `MergeConfig`. Sort `m_layers` by priority once on add.
