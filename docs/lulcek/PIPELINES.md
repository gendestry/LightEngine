# Runtime Pipelines

LightEngine has exactly **two** things that move at runtime:

1. the **render pipeline** — `Engine::update(dt)`, run every frame, and
2. the **command pipeline** — `Engine::command(line)`, run per keystroke/line.

They meet at the *programmer layer*: commands mutate stored state and the
programmer; the render loop reads it all and paints DMX.

---

## 1. The render pipeline (per frame)

```mermaid
sequenceDiagram
    autonumber
    participant App as Caller (~40 Hz)
    participant E as Engine
    participant T as TimeContext
    participant P as Patch / Universes
    participant L as Layers (low→high)
    participant F as Frame
    participant Fx as Fixtures
    participant O as DMXOutput (sACN)

    App->>E: update(dt)
    E->>T: now += dt; ++frame
    E->>P: blackout()  (zero every buffer)
    E->>F: clear()
    E->>L: stable_sort by priority()
    loop each enabled layer
        E->>L: apply(Frame, TimeContext)
        L->>F: contribute(fid, values, HTP|LTP)
    end
    loop each fid in Frame
        E->>Fx: Resolve(mergedValues)
        Fx->>P: write DMX bytes into buffer
    end
    alt output enabled (setIP called)
        E->>O: sendAll(patch)
        O-->>App: sACN packets on the wire
    end
    E->>P: clearDirty()
```

### Why it composes: the merge

Layers are sorted **low → high priority** and applied in that order, so a
higher-priority layer's write lands *on top of* a lower one's. The combine rule
per value is the `MergePolicy`:

```mermaid
flowchart LR
    subgraph contribute["Frame::contribute(fid, in, policy)"]
        direction TB
        c1{"in.color present?"}
        c2{"no existing<br/>OR policy == LTP?"}
        c3{"in.color.v ><br/>existing.v?"}
        take["dst.color = in.color"]
        keep["keep existing"]
        c1 -->|yes| c2
        c2 -->|yes| take
        c2 -->|no HTP| c3
        c3 -->|yes| take
        c3 -->|no| keep
    end
```

| Policy | Meaning | Used for |
|--------|---------|----------|
| **HTP** | Highest-Takes-Precedence (max wins) | intensity |
| **LTP** | Latest/priority-Takes-Precedence (last wins) | color, position |

Consequence — *"programmer over playback" falls out for free*: grab a fixture on
the programmer (priority 1000) and its LTP color overrides a running cue; release
it and the lower layer shows through, untouched, because neither layer clobbered
the other's *stored* intent — only the transient Frame decided who won.

> ⚠️ **Design smell:** color and intensity are bundled in a single `HSV`
> (`FixtureValues::color`). HTP therefore compares `V` and drags hue/sat along
> with the brighter intensity. Proper consoles keep intensity (HTP) and color
> (LTP) on *independent* channels. The ENGINE.md roadmap acknowledges this
> ("split `FixtureValues` into per-category slots"). See
> [ASSESSMENT](ASSESSMENT.md#a2).

### The virtual-dimmer rule (inside `ColorCell::Resolve`)

```mermaid
flowchart TD
    start["ColorCell::Resolve()"]
    q{"has a real<br/>DIMMER param?"}
    real["dimmer.Write(V)<br/>color rendered at full V=1"]
    virt["fold V into the color<br/>(virtual dimmer)"]
    rgb["HSV→RGB → r/g/b.Write()"]
    white["w.Write((1 - s) * V) - naive RGBW"]
    start --> q
    q -->|yes| real --> rgb
    q -->|no| virt --> rgb
    rgb --> white
```

This is the single place the HSV→DMX conversion lives — a genuinely nice
consolidation.

---

## 2. The command pipeline (per line)

```mermaid
sequenceDiagram
    autonumber
    participant U as User
    participant E as Engine
    participant CP as CommandParser
    participant TK as Tokenizer (tokens.txt)
    participant GR as Grammar Engine (commands.syn)
    participant AB as AstBuilder
    participant CE as CommandExecutor
    participant Prog as ProgrammerLayer / Stored

    U->>E: command("1 thru 4 + 7 at 50")
    E->>CP: parse(line)
    CP->>TK: lex → tokens (drop WHITESPACE)
    CP->>GR: parse(startRule) → CST Node
    alt parse failed
        GR-->>CP: null
        CP-->>E: empty Program
        E-->>U: false
    else success
        CP->>AB: build(CST) → Macros::Program
        CP-->>E: Program
        E->>CE: run(Program)
        loop each Command
            CE->>CE: accept(visitor) → visit(SelectCmd/...)
            CE->>Prog: select() / setIntensity() / storeGroup() / recall()
        end
        E-->>U: true
    end
```

### Grammar → AST → action mapping

```mermaid
flowchart LR
    subgraph grammar["commands.syn (EBNF)"]
        g1["selection at?"]
        g2["store modsel"]
        g3["delete modsel"]
        g4["clear"]
        g5["at (NUM | presetsel)"]
    end
    subgraph ast["Macros AST"]
        a1[SelectCmd]
        a2[StoreCmd]
        a3[DeleteCmd]
        a4[ClearCmd]
        a5[AtCmd]
    end
    subgraph act["Engine facade"]
        e1["select(fids) + applyAt"]
        e2["storeGroup / storeColorPreset / storeDimmerPreset"]
        e3["pool.remove"]
        e4["engine.clear()"]
        e5["setIntensity / recallPreset"]
    end
    g1-->a1-->e1
    g2-->a2-->e2
    g3-->a3-->e3
    g4-->a4-->e4
    g5-->a5-->e5
```

**Selector semantics** (`CommandExecutor::resolveSelection`): items apply left to
right, `+` unions (dedup, order-preserving), `-` subtracts. Preset addressing is
`bank.number` where **bank 1 → dimmer**, **bank 2 → color**.

> ⚠️ `AstBuilder` `throw`s `std::runtime_error` on an unexpected node shape, and
> nothing in `Engine::command()` catches it — a grammar/spec mismatch becomes an
> uncaught exception rather than a graceful `false`. See
> [ASSESSMENT](ASSESSMENT.md#a5).

---

## Where state actually lives

A useful cross-cut: nothing durable lives in the Frame or the fixtures. It's all
in **layers** and **pools**.

```mermaid
flowchart TB
    subgraph durable["Durable state"]
        prog["ProgrammerLayer.edits<br/>(FID → FixtureValues)"]
        pools["Stored pools<br/>Groups · ColorPresets · DimmerPresets"]
        patchmap["Patch<br/>FID → fixture, universes"]
    end
    subgraph transient["Transient (rebuilt each tick)"]
        frame["Frame (FID → merged values)"]
        dmx["Universe DMX buffers"]
    end
    prog -->|apply| frame
    pools -->|recall onto| prog
    frame -->|Resolve| dmx
    patchmap -->|Resolve target| dmx

    classDef d fill:#1b5e20,stroke:#66bb6a,color:#fff;
    classDef t fill:#e65100,stroke:#ffb74d,color:#fff;
    class prog,pools,patchmap d;
    class frame,dmx t;
```
