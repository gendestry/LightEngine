# LightEngine — How It Works

A small, layered lighting engine: patch fixtures into DMX universes, drive them
through composable layers, resolve everything to raw DMX each frame, and stream
it out over sACN.

See [`engine.puml`](engine.puml) for the class diagram (green = implemented,
orange = planned).

---

## The big picture

```
layers (persistent state)  --apply()-->  Frame (transient)  --Resolve()-->  DMX buffer  --sACN-->  wire
```

Every frame, `Engine::update(dt)` does five things:

1. **advance the clock** — bump `TimeContext` (`now`, `dt`, `frame`)
2. **blackout** — zero every universe buffer
3. **compose** — each layer, in priority order (low → high), writes its
   contribution into a fresh `Frame`
4. **resolve** — push the merged per-fixture values into each fixture's DMX bytes
5. **output** — if an IP is configured, stream every universe over sACN

The model is **non-tracking** (the frame is rebuilt from scratch every tick) and
**stateless at the fixture level** (fixtures keep no values between frames — all
state lives in the layers).

---

## Layer 1 — GDTF definition (`include/LightEngine/GDTF/`)

The shared, immutable description of a channel. These are *flyweights*: many
fixtures share one `const LogicalChannel` via `shared_ptr`.

- **`Attribute`** — what a channel controls (`DIMMER`, `COLOR_R/G/B/W`, …).
- **`DMXChannel`** — the address + resolution (8- or 16-bit).
- **`Ranges` / `ChannelFunction`** — maps a DMX range (`from..to`) to a physical
  range (`min..max`).
- **`LogicalChannel`** — one attribute at one offset, with its functions.

## Layer 2 — Runtime fixture model (`include/LightEngine/Fixture/`)

Live objects that point into a universe's byte buffer. They own no bytes.

- **`Parameter`** — a runtime view over one `LogicalChannel`. `Write(norm)` takes
  a normalized `0..1` value, finds the matching function, converts to DMX, and
  writes it into the buffer at `baseOffset + address`.
- **`ColorCell`** — a view over the color parameters of one emitter/pixel (the
  ones sharing a `cellIndex`). It is the single place the **HSV → RGB + virtual
  dimmer** rule lives:
  - if the cell has a real `DIMMER` param → intensity (V) goes there, color at full;
  - otherwise V is folded into the color (a *virtual* dimmer).
- **`Fixture`** — one patched light. Holds its parameters flat, an
  attribute index, and its color cells. It is a **stateless sink**:
  `Resolve(const FixtureValues&)` writes whatever it's handed and keeps nothing.

Build order: `Add()` every parameter, then `Build()` to construct the index and
color cells. Placement (buffer + start address) happens when it's added to a
universe.

## Layer 3 — DMX plumbing (`include/LightEngine/DMX/`)

- **`Universe`** — one 512-channel universe. It *is* a `FragmentedStorage`, so it
  owns the byte buffer and packs fixtures into it, wiring each fixture's
  parameters to the buffer. `buffer()` exposes the raw frame for output.
- **`FixtureGroup`** — a named **selection** of fixtures (it holds no values).
  Caches parameters-by-attribute and a flattened color-cell list. You drive it by
  handing it to `ProgrammerLayer::select()`.

---

## The frame pipeline (`include/LightEngine/Engine/Frame.h`)

- **`FixtureValues`** — one fixture's contribution for a frame: an optional
  `color` (HSV) plus a `generic` attribute→float map. Everything optional — a
  layer only fills what it controls.
- **`MergePolicy`** — how overlapping contributions combine:
  - `HTP` (Highest-Takes-Precedence) — intensity: max wins.
  - `LTP` (Latest-Takes-Precedence) — color/position: last (highest priority) wins.
- **`Frame`** — the per-frame value buffer, keyed by FID. `contribute(fid, vals,
  policy)` merges one layer's values in. Rebuilt (`clear()`) every tick — it
  stores nothing durable.
- **`TimeContext`** — the clock: `now`, `dt`, `frame`. Threaded into every
  layer's `apply()` so time-based layers (fades, effects) sample a consistent
  "now".

---

## Layers (`include/LightEngine/Engine/Layer.h`)

This is where state lives, and the reason the model composes cleanly.

```cpp
class Layer {
  virtual int  priority() const = 0;          // low -> high
  virtual bool enabled() const;
  virtual void apply(Frame&, const TimeContext&) = 0;
};
```

Each layer **stores its own values** and writes them into the `Frame` during
`apply()`. Because layers are composed in priority order, a higher-priority
layer's LTP writes land on top of a lower one's — so "programmer over playback"
falls out for free: grab a fixture on the programmer and it overrides a running
cue; release it and the cue shows through, untouched.

Implemented:

- **`ProgrammerLayer`** (priority 1000) — the live editing layer. Holds
  `edits: map<fid, FixtureValues>` and a `selection`. API: `select(group)`,
  `setColor`, `setHueSat`, `setIntensity`, `setIntensityRamp`, `clear`.

Planned:

- **`PlaybackLayer`** (priority 100) — cue/sequence playback. Holds the stored
  `Sequence`s (inert data) plus a live fade cursor (`from`, `target`,
  `fadeElapsed`) driven by `time.dt`.
- **`EffectEngine`** (priority 2000) — time-based effects (e.g. `GradientEffect`)
  applied over a group.

---

## The Engine (`include/LightEngine/Engine/`)

- **`Patch`** — owns the universes and the FID → fixture map. `patch()` copies a
  fixture definition into a universe (auto- or fixed-placed) and assigns FIDs.
  Also tracks a *dirty* set of universes (not yet wired to output).
- **`DMXOutput`** — the transmit stage. One `SacnSender` per universe, lazily
  created. Reads each universe's 512-byte frame (zero copy) and sends it. sACN
  only for now; the seam (`m_senders`) is where an sACN/Art-Net/raw-DMX interface
  would go.
- **`Engine`** — the orchestrator. Owns the patch, groups, the frame, the
  programmer, the layer list, the clock, and the output. `update(dt)` runs the
  five-step loop above.

Output is opt-in: `update()` renders but sends nothing until you call `setIP()`
(so headless/print-only runs stay offline). `Interfaces::primaryIP()` finds this
machine's outbound address.

---

## A minimal session

```cpp
Engine::Engine engine;

// patch 93 RGB fixtures into universe 8
auto fids = engine.patch(MakeRGB(), 8, 93);
engine.addToGroup("all", fids);

// set them all to yellow at full via the programmer layer
auto& prog = engine.programmer();
prog.select(*engine.getGroup("all"));
prog.setColor(Utils::Colors::HSV(60.f, 1.f, 1.f));

// stream sACN from the primary interface
engine.setIP(Utils::Network::Interfaces::primaryIP());

// drive frames (continuous, ~40 Hz)
for (;;) { engine.update(dt); /* sleep */ }
```

---

## Planned subsystems

- **Playback / Show** — `Cue` (a per-fixture `FixtureValues` snapshot + fade) and
  `Sequence` (an ordered cuelist), driven by `PlaybackLayer`.
- **Effects** — `EffectEngine` + `Effect`/`GradientEffect`, contributing on top.
- **Palettes** — named, **referenced**, category-scoped values (dimmer / color /
  position / beam). Layers store a `PaletteRef` (absolute *or* palette id) and
  **dereference every frame**, so editing a palette live-updates everything that
  points at it. Depends on:
  - splitting `FixtureValues` into per-category slots (intensity / color /
    position / beam), and
  - a **`Classifier`** mapping `Attribute → category`.
- **FixtureLibrary / GDTF import** — load real fixtures from `.gdtf` files instead
  of hand-building them.

---

## Design invariants (don't break these)

1. **Fixtures are stateless.** They render what they're handed and forget. State
   belongs in layers.
2. **The Frame is transient.** Rebuilt every tick; never persist through it.
3. **State is per-layer.** Two layers may hold different values for the same
   fixture — the `Frame` merge decides who wins, without either clobbering the
   other's stored intent.
4. **Output is a pure sink.** It reads universe buffers; it never mutates state.
