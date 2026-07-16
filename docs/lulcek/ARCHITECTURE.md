# Architecture — Zoom Levels

This document walks from the outside in: **context → modules → components →
classes**, then covers **ownership & lifetime**. Each level adds detail without
contradicting the one above it.

- [Level 1 — Context](#level-1--context) *(see [README](README.md))*
- [Level 2 — Layered modules](#level-2--layered-modules)
- [Level 3 — Components per module](#level-3--components-per-module)
- [Level 4 — Class diagrams](#level-4--class-diagrams)
- [Ownership & lifetime](#ownership--lifetime)

---

## Level 2 — Layered modules

The codebase is a clean **dependency stack**: higher layers depend on lower
ones, never the reverse. GDTF is the immutable foundation; Commands is the thin
text skin on top.

```mermaid
flowchart TD
    CMD["<b>Commands</b><br/>CommandParser · AstBuilder<br/>CommandExecutor · Macros AST"]
    ENG["<b>Engine</b><br/>Engine · Patch · DMXOutput<br/>Layer/ProgrammerLayer · Frame · TimeContext<br/>Pools (Stored, Group, Preset)"]
    DMX["<b>DMX</b><br/>Universe · FixtureGroup"]
    FIX["<b>Fixture</b><br/>Fixture · Parameter · ColorCell"]
    GDTF["<b>GDTF</b><br/>Attribute · LogicalChannel<br/>DMXChannel · Ranges"]

    UTILS["<i>Utils</i><br/>Colors (HSV/RGB) · Network (IP/SACN)<br/>FragmentedStorage"]
    SYN["<i>NewSyntaxParser</i><br/>Tokenizer · Grammar · CST Node"]

    CMD --> ENG
    CMD --> SYN
    ENG --> DMX
    ENG --> FIX
    ENG --> UTILS
    DMX --> FIX
    DMX --> UTILS
    FIX --> GDTF
    FIX --> UTILS

    classDef ext fill:#37474f,stroke:#90a4ae,color:#fff;
    class UTILS,SYN ext;
```

**Reading it:** a change to a GDTF definition can ripple up through everything;
a change to Commands touches nothing below Engine. That directionality is the
single most important structural property of the project — it is respected
consistently.

### Responsibilities

| Module | Role | Key insight |
|--------|------|-------------|
| **GDTF** | Immutable, shared channel *descriptions* | Flyweights — many fixtures share one `const LogicalChannel` via `shared_ptr` |
| **Fixture** | Live objects that *point into* a universe buffer | Own **no bytes**; stateless sinks (`Resolve()` writes and forgets) |
| **DMX** | The 512-channel universe + named selections | `Universe` **is-a** `FragmentedStorage`; `FixtureGroup` is a pure selection |
| **Engine** | Orchestration: patch → compose → resolve → output | Non-tracking render loop; state lives in **layers** and **pools** |
| **Commands** | Text → typed AST → Engine actions | Decoupled parse/build/execute via the visitor pattern |

---

## Level 3 — Components per module

### Engine internals

```mermaid
flowchart TB
    subgraph Engine
        patch[Patch<br/>universes + FID→fixture map<br/>dirty set]
        time[TimeContext<br/>now · dt · frame]
        frame[Frame<br/>FID→FixtureValues<br/>rebuilt each tick]
        prog[ProgrammerLayer<br/>priority 1000]
        layers["m_layers: vector&lt;Layer*&gt;<br/>composed low→high"]
        stored[Stored<br/>object pools]
        output[DMXOutput<br/>sACN senders]
        parser[CommandParser + CommandExecutor<br/>pImpl via unique_ptr]
    end

    prog -. registered in .-> layers
    layers -->|apply| frame
    frame -->|Resolve| patch
    patch -->|buffers| output
    prog -->|resolves FIDs| patch
    stored -->|recall onto| prog
    parser -->|drives facade| Engine
```

### Pools (the show's stored objects)

```mermaid
flowchart LR
    stored[Stored]
    pg["Pool&lt;Group&gt;"]
    pc["Pool&lt;ColorPreset&gt;"]
    pd["Pool&lt;DimmerPreset&gt;"]
    seq["Pool&lt;Sequence&gt;  (planned)"]:::todo
    eff["Pool&lt;Effect&gt;  (planned)"]:::todo

    stored --> pg
    stored --> pc
    stored --> pd
    stored -.-> seq
    stored -.-> eff

    classDef todo fill:#e65100,stroke:#ffb74d,color:#fff,stroke-dasharray: 4 3;
```

Each `Pool<T>` is a numbered slot map (`uint32_t → shared_ptr<T>`) with a
name index, `nextFree()` gap-finding, and `store/get/remove/move/rename`.

### Command subsystem

```mermaid
flowchart LR
    txt["command line<br/><i>'1 thru 4 + 7 at 50'</i>"]
    tok[Tokenizer<br/>tokens.txt]
    cst["CST<br/>Parsing::Syntax::Node"]
    ast["typed AST<br/>Macros::Program"]
    exec[CommandExecutor<br/>CommandVisitor]
    eng[Engine facade]

    txt --> tok --> cst
    cst -->|AstBuilder| ast
    ast -->|accept/visit| exec
    exec --> eng
```

The grammar (`commands.syn`) and token defs (`commands.txt`) are **external data
files**, so the console language can evolve without recompiling.

---

## Level 4 — Class diagrams

### Fixture rendering path (GDTF → Fixture → DMX)

```mermaid
classDiagram
    direction LR

    class LogicalChannel {
        +Attribute attribute
        +DMXChannel channel
        +vector~ChannelFunction~ functions
    }
    class DMXChannel {
        +uint16 address
        +Resolution res
        +MaxDMX() uint16
    }
    class ChannelFunction {
        +DMXRange range_dmx
        +PhysicalRange range_physical
        +Evaluate(dmx) float
    }

    class Parameter {
        -shared_ptr~LogicalChannel~ def
        -uint8ptr bytes
        -uint32 baseOffset
        -uint16 cellIndex
        +Write(norm) void
    }
    class ColorCell {
        -Parameter rgbw
        -Parameter dimmer
        -HSV hsv
        +Resolve() void
    }
    class Fixture {
        -vector~Parameter~ parameters
        -map~Attribute,Params~ byAttribute
        -vector~ColorCell~ colorCells
        +Add(def, cell) Parameter
        +Build() void
        +Resolve(FixtureValues) void
    }

    LogicalChannel *-- DMXChannel
    LogicalChannel *-- ChannelFunction
    Parameter ..> LogicalChannel : shares const
    ColorCell o-- Parameter : views
    Fixture *-- Parameter
    Fixture *-- ColorCell
    Fixture ..> LogicalChannel : Add
```

### Composition core (layers, frame, engine)

```mermaid
classDiagram
    direction TB

    class Layer {
        <<abstract>>
        +priority() int
        +enabled() bool
        +apply(Frame, Time) void
    }
    class ProgrammerLayer {
        -FixtureGroup selection
        -map~FID,FixtureValues~ edits
        +select(group) void
        +setColor(hsv) void
        +priority() int
    }
    class Frame {
        -map~FID,FixtureValues~ values
        +contribute(fid, vals, policy) void
        +get(fid) ValuesPtr
        +clear() void
    }
    class FixtureValues {
        +optional~HSV~ color
        +map~Attribute,float~ generic
    }
    class MergePolicy {
        <<enum>>
        HTP
        LTP
    }
    class Engine {
        -Patch patch
        -Frame frame
        -ProgrammerLayer programmer
        -vector~LayerPtr~ layers
        -Stored stored
        -DMXOutput output
        -TimeContext time
        +patch(fixture, uni, n)
        +update(dt) void
        +command(line) bool
    }

    Layer <|-- ProgrammerLayer
    Engine o-- Layer : composes low to high
    Engine *-- Frame
    Engine *-- ProgrammerLayer
    Frame *-- FixtureValues
    Frame ..> MergePolicy
    ProgrammerLayer ..> Frame : apply
```

### Stored objects (pools & presets)

```mermaid
classDiagram
    direction TB

    class PoolObject {
        -uint32 number
        -string name
        +describe() string
    }
    class Pool~T~ {
        -map~uint32,PtrT~ items
        -map~string,uint32~ byName
        +store(num, obj) T
        +get(num) PtrT
        +move(from, to) bool
        +nextFree() uint32
    }
    class Group {
        -FixtureGroup fg
        +fids() vector~uint16~
    }
    class PresetBase {
        <<abstract>>
        +PresetType type
        +recall(prog, selection) void
    }
    class Preset~T~ {
        -map~FID,T~ values
        +set(fid, v) void
        +appliesTo(fid) bool
    }
    class ColorPreset
    class DimmerPreset

    PoolObject <|-- Group
    PoolObject <|-- PresetBase
    PresetBase <|-- Preset
    Preset <|-- ColorPreset
    Preset <|-- DimmerPreset
    Pool o-- PoolObject : holds shared_ptr
```

### Command AST (visitor pattern, auto-generated)

```mermaid
classDiagram
    direction TB

    class Command {
        <<abstract>>
        +accept(visitor) void
    }
    class Selector {
        <<abstract>>
        +accept(visitor) void
    }
    class SelectCmd {
        +vector~Item~ items
        +bool hasAt
        +AtValue at
    }
    class StoreCmd
    class DeleteCmd
    class ClearCmd
    class AtCmd
    class Fixture_ {
        +int64 id
    }
    class FixtureRange {
        +int64 from
        +int64 to
    }
    class Group_ {
        +int64 id
    }
    class Preset_ {
        +int64 bank
        +int64 number
    }
    class CommandExecutor {
        -Engine engine
        +run(program) void
        +visit(cmd) void
    }

    Command <|-- SelectCmd
    Command <|-- StoreCmd
    Command <|-- DeleteCmd
    Command <|-- ClearCmd
    Command <|-- AtCmd
    Selector <|-- Fixture_
    Selector <|-- FixtureRange
    Selector <|-- Group_
    Selector <|-- Preset_
    CommandExecutor ..|> Command : CommandVisitor
    SelectCmd o-- Selector : items
```

> The `Macros::` AST is **generated** from `commands.spec` by `gen_ast.py`
> (header carries an "AUTO-GENERATED — do not edit" banner). The grammar,
> tokens, and AST spec are three coordinated data files.

---

## Ownership & lifetime

Who owns the heap, and who merely *points*, is the crux of understanding this
engine. Getting it wrong is the main safety risk (see [ASSESSMENT](ASSESSMENT.md)).

```mermaid
flowchart TD
    engine[Engine]
    patch[Patch]
    uni["Universe (value in map)"]
    buf["512-byte DMX buffer<br/>owned by Universe"]
    fixptr["shared_ptr&lt;Fixture&gt;"]
    param["Parameter (bytes* → buffer)"]
    stored[Stored / Pools]
    group[Group → FixtureGroup]
    prog[ProgrammerLayer]

    engine -->|owns| patch
    engine -->|owns| stored
    engine -->|owns| prog
    patch -->|owns by value| uni
    uni -->|owns| buf
    uni -->|owns| fixptr
    patch -->|shares| fixptr
    fixptr --> param
    param -.non-owning ptr.-> buf
    stored --> group
    group -.shares.-> fixptr
    prog -.shares in selection.-> fixptr

    classDef own fill:#1b5e20,stroke:#66bb6a,color:#fff;
    classDef ref fill:#0d47a1,stroke:#64b5f6,color:#fff;
    class buf,uni own;
    class param,group,prog ref;
```

**Key facts**

- A **`Universe`** owns its 512-byte buffer *and* the `shared_ptr<Fixture>`s placed in it.
- A **`Fixture`**'s `Parameter`s and `ColorCell`s hold **raw, non-owning pointers**
  into that buffer and into the fixture's own parameter vector.
- `FixtureGroup`, `Group`, and `ProgrammerLayer::selection` **share** fixtures
  (`shared_ptr`) but own none — they are pure selections.
- `m_layers` is a `vector<Layer*>` of **raw** pointers; the programmer is a member,
  external layers added via `addLayer()` are borrowed (no ownership transfer).

> ⚠️ Because `Parameter::bytes` and `ColorCell` pointers are non-owning and are
> re-wired on placement/copy, fixture **copy semantics matter**. `Fixture`
> correctly rebuilds its index in the copy ctor / assignment so pointers aim at
> *its own* parameters. This is the subtle bit the code gets right.
