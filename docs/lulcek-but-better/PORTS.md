# Commands, Ports & Adapters — The Catalogue

The engine (`domain/` + `app/`) exposes exactly two kinds of seam:

- an **inbound command bus** — the one entry point every front-end calls
  (`Engine::dispatch(const Command&)`), plus a read-model query;
- a set of **outbound ports** — interfaces the engine *owns* and the outside
  world *implements* (`DmxSink`, `Clock`, `FixtureLibrary`, `ShowStore`).

An **adapter** is a driven outer-ring class that implements an outbound port; a
**front-end** (`api/`) is a driving outer-ring class that builds `Command`s and
calls `dispatch`. This document lists the command vocabulary, every port with a
concrete C++ signature, the adapters that fulfil them, and the **composition
root** that wires everything.

The signatures are illustrative targets, not final API — they exist to make the
boundaries unambiguous and to show that the design is buildable in C++23.

- [Result & Error](#result--error)
- [Inbound — the Command bus](#inbound--the-command-bus)
- [Outbound ports](#outbound-ports)
- [Adapters (driven)](#adapters-driven)
- [Front-ends (api/, driving)](#front-ends-api-driving)
- [Composition root — EngineBuilder](#composition-root--enginebuilder)
- [Directory & CMake layout](#directory--cmake-layout)

---

## Result & Error

Every fallible operation returns a `Result<T>` instead of a bare `bool`, a
silently-empty value, or an uncaught `throw` (fixes `../lulcek` A5). Uses
`std::expected` (C++23, already the project standard).

```cpp
namespace LightEngine::App {

enum class ErrorCode {
    Ok, ParseError, UnknownFixture, UnknownGroup, UnknownPreset,
    UnknownPersonality, AddressClash, UniverseFull, NoSelection,
    NotLoaded, IoError,
};

struct Error {
    ErrorCode   code;
    std::string message;               // human-readable, safe to show a user
    std::optional<std::size_t> at;     // source position for parse errors
};

template <class T>
using Result = std::expected<T, Error>;

using Status = Result<std::monostate>; // for outcome-less operations
} // namespace LightEngine::App
```

> **Where errors are rendered.** `Error` is a *value*, not text on stdout. The
> engine returns it; a **front-end** turns it into console text, a GUI dialog,
> or a serialized error frame. Parsing errors (`ParseError`, with `at`) originate
> in the console front-end's parser and never reach the engine at all.

---

## Inbound — the Command bus

The engine's **entire** inbound surface. There is no `ConsolePort`, no per-verb
public API, and no `Engine&` for a driver to reach into — only a serializable
`Command` and one `dispatch`. This is the seam that breaks the `../lulcek`
command→engine coupling: nothing outside the engine calls a domain method
directly; everything is expressed as data.

### The `Command` vocabulary

A `Command` is a **plain, serializable value** describing one operator intent.
It carries no behaviour, references only stable value types, and is trivially
(de)serialized — which is exactly what lets a network GUI send one over the
wire, a show file record a stream of them, or a test author write one by hand.

```cpp
namespace LightEngine::App {

// --- Selection & programming (scoped to the live selection) --------------
struct Select        { Selector sel; };          // replaces the selection
struct AddToSel      { Selector sel; };           // unions
struct SubtractSel   { Selector sel; };           // removes
struct Deselect      {};
struct SetIntensity  { float level01; };
struct SetColor      { Domain::ColorValue color; };
struct SetIntensityRamp { float a, b; };
struct FanColor      { float hueA, hueB; float sat = 1.f; };

// --- Patch (engine resolves the personality by NAME via FixtureLibrary) --
struct Patch         { std::string personality;   // e.g. "RGB"
                       UniverseId  universe;
                       uint16_t    count;
                       PatchOptions opts = {}; };

// --- Stored objects ------------------------------------------------------
struct StoreGroup    { std::optional<uint32_t> number; };
struct StorePreset   { PresetKind kind; std::optional<uint32_t> number; };
struct RecallGroup   { uint32_t number; };
struct RecallPreset  { PresetKind kind; uint32_t number; };
struct RemoveObject  { ObjectRef ref; };
struct RenameObject  { ObjectRef ref; std::string name; };

// --- Console-style staged clear -----------------------------------------
struct Clear         {};                          // wipe values + selection

using Command = std::variant<
    Select, AddToSel, SubtractSel, Deselect,
    SetIntensity, SetColor, SetIntensityRamp, FanColor,
    Patch, StoreGroup, StorePreset, RecallGroup, RecallPreset,
    RemoveObject, RenameObject, Clear>;
} // namespace LightEngine::App
```

`Selector` is a small value type the front-end builds (a fixture id, a range, a
group ref, unioned/subtracted). The engine resolves it against the `Rig` — the
front-end never touches a fixture or a fid map. `ObjectRef`, `PresetKind`, and
`PatchOptions` are likewise plain value types owned by `app/`.

### The `Engine` interface

```cpp
namespace LightEngine::App {

// What a Command produces on success. A small closed variant keeps the bus
// honest: the front-end matches on it to render/serialize the outcome.
struct Ack     {};                                   // generic success
struct Stored  { uint32_t number; };                 // Store{Group,Preset}
struct Patched { std::vector<FixtureId> ids; };      // Patch

using CommandResult = std::variant<Ack, Stored, Patched>;

// The engine's inbound facade. One command entry point, one read-model query,
// one per-tick call. Knows nothing about text, grammar, sockets, or files.
struct Engine {
    virtual ~Engine() = default;

    // Execute one intent. All programming/patch/store/recall flows come here.
    virtual Result<CommandResult> dispatch(const Command& cmd) = 0;

    // Read model (query side): an immutable snapshot of what is on stage.
    // Front-ends render it to console text, a GUI grid, or serialized bytes.
    virtual const Domain::FrameState& state() const = 0;

    // Per-tick orchestration: clock.sample → compose → render → sink.send.
    // The host calls this ~40 Hz; it is not a Command (no operator issues it).
    virtual void renderTick() = 0;
};
} // namespace LightEngine::App
```

> **Why a command bus and not typed verbs?** (The trade-off, chosen
> deliberately.) One `dispatch(Command)` keeps the engine's inbound surface at a
> single, uniform, *serializable* point: every front-end — CLI, GUI, a network
> client, a replayed show file — speaks the exact same vocabulary, and the
> executor becomes a pure `AST → Command` mapper testable with zero engine. The
> cost is that `dispatch` fans out on a `std::variant` (an `std::visit` switch)
> and the success payload is a small `CommandResult` variant rather than a
> statically-typed return per verb. For this domain — a lighting desk is a
> *stream of operator commands* — that shape is the natural fit, and it is what
> makes save/load, undo, remote control, and macro recording fall out for free.

> **A note on queries.** `state()` is the read side. If richer read models are
> needed later (pool listings, patch sheets), add more `const` query methods to
> `Engine` (or a sibling `EngineQuery` interface a front-end depends on) — they
> return domain value snapshots, never mutable handles, so a front-end can format
> or serialize but never mutate the engine behind `dispatch`'s back.

---

## Outbound ports

The things the engine needs *done for it*. Each is **as generic as the domain
allows**: the engine hands over a domain value and names an intent; the adapter
does the translation into its own world (packets, files, syscalls, GDTF XML).

### `DmxSink` — the wire

```cpp
namespace LightEngine::App {

// The engine hands over an immutable, fully-rendered domain frame; the adapter
// translates it to a transport. Pure domain data in, I/O hidden.
struct DmxSink {
    virtual ~DmxSink() = default;
    virtual void configure(const SinkConfig& cfg) = 0;   // source name, iface
    virtual Status send(const Domain::DmxFrame& frame) = 0;
};
} // namespace LightEngine::App
```

Replaces `DMXOutput`'s concrete sACN surface (`send`/`update`/`sendAll` over
`SacnSender`). The engine calls `send(frame)` once per tick with a `DmxFrame`
(domain value); **which** universes go out, over **which** protocol, and in
what packet format is entirely the adapter's translation job.

### `Clock` — time

```cpp
namespace LightEngine::App {

struct TimeContext { double now = 0; float dt = 0; uint64_t frame = 0; };

struct Clock {
    virtual ~Clock() = default;
    virtual TimeContext sample() = 0;   // advance & return the tick's time
};
} // namespace LightEngine::App
```

Formalises today's already-deterministic `update(dt)` (G3). `SystemClock` reads
`steady_clock`; `ManualClock` is the test fake (advance by hand).

### `FixtureLibrary` — personalities

```cpp
namespace LightEngine::App {

// The engine asks by NAME (as carried in a Patch command); the adapter
// translates its source format (builder calls, GDTF zip/XML) into a domain
// Personality. The engine never parses GDTF.
struct FixtureLibrary {
    virtual ~FixtureLibrary() = default;
    virtual Result<Domain::Personality> get(const std::string& name) = 0;
    virtual std::vector<std::string> names() const = 0;
};
} // namespace LightEngine::App
```

Turns the empty `FixtureLibrary` class (`../lulcek` A12) into a real seam. The
name-based `patch(...)` that today's `Patch` has commented out (`Patch.h:45`)
becomes: `dispatch(Patch{"RGB", …})` → service asks the library for the
`Personality` → patches it into the `Rig`.

### `ShowStore` — persistence (new)

```cpp
namespace LightEngine::App {

struct ShowStore {
    virtual ~ShowStore() = default;
    virtual Status               save(const Domain::Show&, std::string_view id) = 0;
    virtual Result<Domain::Show> load(std::string_view id) = 0;
};
} // namespace LightEngine::App
```

Enables save/load with **zero** domain change — the `Show` aggregate is
serialised by an adapter, not by teaching the domain about files. (A serialized
*stream of `Command`s* is a complementary option for macro/undo, handled the
same way: by a front-end/adapter, never the engine.)

---

## Adapters (driven)

The outer ring on the **driven** side. Each implements one outbound port and
does the domain⇄external translation. Note the command parser is **not** here
any more — it is a front-end (next section).

| Adapter | Port | Wraps / uses | Replaces today's |
|---------|------|--------------|------------------|
| `SacnDmxSink` | `DmxSink` | `Utils::Network::SACN`, `IP` | `Engine::DMXOutput` |
| `ArtNetDmxSink` *(planned)* | `DmxSink` | Art-Net UDP | — (the TODO in `DMXOutput.h`) |
| `RecordingDmxSink` *(test)* | `DmxSink` | in-memory `vector<DmxFrame>` | — |
| `SystemClock` | `Clock` | `std::chrono::steady_clock` | inline in caller loop |
| `ManualClock` *(test)* | `Clock` | a member counter | — |
| `BuilderFixtureLibrary` | `FixtureLibrary` | `FixtureBuilder` | hand-built fixtures in `main` |
| `GdtfFixtureLibrary` *(planned)* | `FixtureLibrary` | GDTF zip/XML parser | the GDTF import TODO |
| `FileShowStore` *(planned)* | `ShowStore` | JSON/binary on disk | — |

---

## Front-ends (api/, driving)

The outer ring on the **driving** side. A front-end owns *all* UI translation:
it turns whatever the operator does into `Command`s, dispatches them, and turns
the engine's `Result` / `state()` back into its own medium. The engine is blind
to which (if any) front-end is attached.

### The console front-end keeps G4 intact

The tokenizer / grammar / generated-AST design (the `../lulcek` G4 strength, and
the `commands.syn` / `commands.txt` / `commands.spec` data files) is **kept
verbatim** — it just moves from `Commands/` into `api/console/`. Its two jobs:

1. **Inbound translation:** `text → CST → AST → Command`. The old
   `CommandExecutor` becomes a pure **AST→`Command` mapper** that calls
   `Engine::dispatch` — it holds `App::Engine&`, never a concrete engine, and
   emits *data*, so it is unit-testable with no engine at all.
2. **Outbound translation:** a `ConsoleView` renders `Engine::state()`
   (`FrameState`) and each `Result<CommandResult>` into terminal text — the
   printing that is scattered through today's `Engine`/`main` lives here.

Grammar/token file locations come from an injected **asset root**, not a
CWD-relative literal (fixes `../lulcek` A6 — `main.cpp`'s
`"include/LightEngine/Commands/data/commands.txt"` only worked from the repo
root).

```cpp
namespace LightEngine::Api {

class ConsoleFrontend {                 // driving front-end (api/)
    App::Engine&             m_engine;  // <-- the seam (was Engine& / ConsolePort&)
    Console::Parser          m_parser;  // tokenizer + grammar + AST builder
    Console::AstToCommand    m_map;     // AST verb -> App::Command
    Console::ConsoleView     m_view;    // FrameState/Result -> terminal text
public:
    ConsoleFrontend(App::Engine& engine, const AssetPaths& assets);

    // Parse a line → Command(s) → dispatch → rendered text (or a rendered error).
    std::string run(std::string_view line);
};
} // namespace LightEngine::Api
```

A GUI front-end (`api/gui/…`) is a *peer*: it builds the same `App::Command`s
from widget events and formats `state()` into a grid — no parser, but the same
engine seam. A network front-end (de)serializes `Command`s and `state()` over a
socket. None of them touches `domain/` or the driven adapters.

---

## Composition root — `EngineBuilder`

The **only** place concrete adapters and front-ends meet the engine. Lives in
`api/`. Everything below it is pure or interface-typed; this is where the wiring
(and the `new`s) happen, and where an embedder chooses protocols, front-ends,
and test doubles.

```cpp
namespace LightEngine::Api {

class EngineBuilder {
    std::unique_ptr<App::DmxSink>        m_sink;
    std::unique_ptr<App::Clock>          m_clock;
    std::unique_ptr<App::FixtureLibrary> m_library;
    std::unique_ptr<App::ShowStore>      m_store;    // optional
public:
    EngineBuilder& withSacnOutput(std::string iface, std::string sourceName);
    EngineBuilder& withSystemClock();
    EngineBuilder& withBuilderLibrary();
    // test overrides
    EngineBuilder& withSink(std::unique_ptr<App::DmxSink>);
    EngineBuilder& withClock(std::unique_ptr<App::Clock>);

    // Constructs an EngineService with the wired outbound ports. The returned
    // host owns the engine + ports; attach front-ends to host.engine().
    EngineHost build();
};

// The public handle an embedder holds. Owns the engine + its driven ports and
// hands out the App::Engine& that front-ends dispatch against.
class EngineHost {
public:
    App::Engine&              engine();          // dispatch / state / renderTick
    const Domain::FrameState& state() const;     // convenience passthrough
    void                      renderTick();       // the ~40 Hz call
};
} // namespace LightEngine::Api
```

A minimal embedder — compare to today's `examples/demo/main.cpp`, which reaches
into `engine.programmer()`, `engine.stored()`, `engine.getUniverse()` directly:

```cpp
auto host = Api::EngineBuilder{}
                .withBuilderLibrary()
                .withSacnOutput("eth0", "LightEngine")
                .withSystemClock()
                .build();

// A console front-end translates text ⇄ Command against the engine seam.
Api::ConsoleFrontend console(host.engine(),
                             Api::AssetPaths::relativeTo(argv[0]));

std::cout << console.run("patch RGB 8 x93");     // -> Command Patch{...} -> dispatch
std::cout << console.run("1 thru 93 at 100");    // -> Select + SetIntensity
for (;;) host.renderTick();                       // clock + compose + render + send
```

The same engine under test, swapping two adapters and skipping the console
entirely — build `Command`s directly (see [TESTABILITY.md](TESTABILITY.md)):

```cpp
auto rec   = std::make_unique<RecordingDmxSink>();
auto* sink = rec.get();
auto host  = Api::EngineBuilder{}
                 .withBuilderLibrary()
                 .withSink(std::move(rec))
                 .withClock(std::make_unique<ManualClock>())
                 .build();

host.engine().dispatch(App::Patch{"RGB", UniverseId{8}, 93});
host.engine().dispatch(App::Select{ Selector::range(1, 93) });
host.engine().dispatch(App::SetIntensity{1.0f});
host.renderTick();
// ... assert on sink->frames().back()
```

---

## Directory & CMake layout

Four link targets make the dependency rule a **build error**, not a convention.
`le_domain` and `le_app` must not link `utils`'s network object or `synparser` —
the parser now lives with the console *front-end*, inside `le_api`.

```
include/LightEngine/            src/LightEngine/
  domain/                         domain/
    fixture/  (Personality, Fixture, Channel, Attribute, AttributeCategory, ColorMixer)
    rig/      (Rig, AddressAllocator, DmxAddress, FixtureId, UniverseId, Selection)
    compose/  (Layer, ProgrammerLayer, Compositor, FrameState, FixtureState, MergePolicy)
    render/   (DmxRenderer, DmxFrame)
    show/     (Show, Pool, Group, Preset, PoolObject)
    programmer/ (Programmer)
  app/                            app/
    command/  (Command, CommandResult, Selector, ObjectRef, PresetKind, PatchOptions)
    ports/    (DmxSink, Clock, FixtureLibrary, ShowStore)        # outbound only
    Engine.h              (inbound interface: dispatch + state + renderTick)
    EngineService.h/.cpp  (implements Engine over the domain aggregates)
    Result.h  Error.h
  adapters/                       adapters/            # driven only
    dmx/      (SacnDmxSink, [ArtNetDmxSink], RecordingDmxSink)
    clock/    (SystemClock, ManualClock)
    library/  (BuilderFixtureLibrary, [GdtfFixtureLibrary])
    store/    ([FileShowStore])
  api/                            api/                 # driving front-ends + root
    console/  (Parser, AstToCommand, ConsoleView, ConsoleFrontend, data/*.syn|txt|spec)
    [gui/]    ([GuiFrontend] — builds Command from widgets)
    EngineBuilder.h/.cpp  EngineHost.h/.cpp  AssetPaths.h
```

```cmake
add_library(le_domain   ${DOMAIN_SRC})
target_link_libraries(le_domain   PUBLIC utils_colors)          # shared kernel only

add_library(le_app      ${APP_SRC})
target_link_libraries(le_app      PUBLIC le_domain)             # no I/O, no parser

add_library(le_adapters ${ADAPTERS_SRC})                        # driven adapters
target_link_libraries(le_adapters PUBLIC le_app PRIVATE utils)

add_library(le_api      ${API_SRC})                             # front-ends + root
target_link_libraries(le_api      PUBLIC le_app le_adapters PRIVATE synparser)

add_library(LightEngine INTERFACE)                              # umbrella
target_link_libraries(LightEngine INTERFACE le_api)
```

> Note the parser dependency (`synparser`) moved to **`le_api`**: it is a
> front-end concern, so neither `le_domain` nor `le_app` links it. That is the
> linker-level statement of "the engine speaks `Command`, not text."

> If `utils` is monolithic today (colors + network in one target), the migration
> either splits it into `utils_colors` / `utils_net` or, as an interim, links
> the whole thing everywhere and relies on include-time discipline + a
> lint/`grep` check until the split lands (see [MIGRATION.md](MIGRATION.md),
> Phase 0).
