# Console design — combining Terminal + Commands with the grammar pipeline

Status: proposal (not yet implemented)

## Where it lives, and why

`Utils` has no dependency on `NewSyntaxParser`, and shouldn't gain one just for
this. So the split is:

- **`Utils::Commands::Console<Ctx>`** — generic, lives in `Utils/Commands/`.
  Knows about `Terminal` and `Registry<Ctx>` only. No knowledge of grammars,
  `Engine`, or `LightEngine` at all — reusable in other repos as-is.
- **The grammar fallback** — `LightEngine`-specific, injected as a plain
  callback. `Console` never includes `CommandParser.h`; it just calls whatever
  `std::function` it was given when nothing else matched.

Same pattern `Registry::addPredicate` already uses internally (a `Condition`
callback it doesn't need to understand).

## `Terminal` needs one small addition first

`Terminal::readInput()` runs an internal loop; on Enter, `handleEnter()`
pushes the line to history and clears the buffer, but never hands the
finished line back out to the caller. There is currently no way for a
dispatch loop to receive what was typed.

```cpp
// Terminal.h — add alongside m_exitCallack
std::function<void(const std::string&)> m_submitCallback;
Terminal(std::function<void()> exitCallback, std::function<void(const std::string&)> submitCallback);

// Terminal.cpp — handleEnter, right before input.clear()
void Terminal::handleEnter(std::string &input)
{
    history.push(input);
    std::cout << '\n';
    std::cout.flush();
    if (m_submitCallback) m_submitCallback(input);   // <- new
    cursor.x = 0;
    input.clear();
}
```

## `Console<Ctx>` header

```cpp
// Utils/Commands/Console.h
#pragma once
#include "Utils/Commands/Registry.h"
#include "Utils/Terminal/Terminal.h"
#include <functional>
#include <string>

namespace Utils::Commands
{
// One REPL, scoped to whatever Ctx represents - a single engine component
// (Patch&) or a coordinating facade (Engine&), same as Registry<Ctx> itself.
// Owns line editing (Terminal) and verb dispatch (Registry<Ctx>); does not
// know what a "grammar" is - an unmatched line falls through to whatever
// `onUnmatched` the caller installed, or is reported unknown if none was.
template <typename Ctx>
class Console
{
public:
    using Reg = Registry<Ctx>;

    Console(Ctx &ctx, std::string prompt = "> ");

    [[nodiscard]] Reg &commands() { return m_registry; }

    // Last-resort handler for a line that matched no keyed or predicate
    // command. Returns false to report "unknown command". A grammar-backed
    // caller (LightEngine::Commands) plugs its parser+executor in here.
    void onUnmatched(std::function<bool(const std::string &line, Ctx &)> handler);

    // Enters Terminal::readInput(); returns when the user exits (Ctrl-D/.quit).
    void run();

private:
    Ctx &m_ctx;
    std::string m_prompt;
    Terminal::Terminal m_terminal;
    Reg m_registry;
    std::function<bool(const std::string &, Ctx &)> m_onUnmatched;

    void onLine(const std::string &line);
};
} // namespace Utils::Commands
```

## `Console<Ctx>` implementation

```cpp
// Utils/Commands/Console.cpp (template - could stay header-only like Registry,
// but Terminal already has a .cpp, so this can too)
#include "Utils/Commands/Console.h"
#include <iostream>

namespace Utils::Commands
{
template <typename Ctx>
Console<Ctx>::Console(Ctx &ctx, std::string prompt)
    : m_ctx(ctx), m_prompt(std::move(prompt)),
      m_terminal([this] { /* exit: nothing extra, readInput() just returns */ },
                 [this](const std::string &line) { onLine(line); })
{
}

template <typename Ctx>
void Console<Ctx>::onUnmatched(std::function<bool(const std::string &, Ctx &)> handler)
{
    m_onUnmatched = std::move(handler);
}

template <typename Ctx>
void Console<Ctx>::onLine(const std::string &line)
{
    if (line.empty())
        return;

    Args args(line);
    auto res = m_registry.resolve(args.command(), args, m_ctx);
    if (res)
    {
        if (!res.cmd->func(args, m_ctx))
            std::cout << "  error: " << res.cmd->key << "\n";
        return;
    }
    if (res.blocked())
    {
        std::cout << "  " << res.reason << "\n";
        return;
    }
    if (m_onUnmatched && m_onUnmatched(line, m_ctx))
        return;

    std::cout << "  unknown command\n";
}

template <typename Ctx>
void Console<Ctx>::run() { m_terminal.readInput(); }
} // namespace Utils::Commands
```

(This needs `resolve(key, args, ctx)` to also try the registry's predicates
when the key lookup fails outright, which it already does — `resolve()`'s
single-key overload calls the `{key}` list form, which falls through to
`m_predicate` after the keyed miss. Good as-is.)

## Wiring it for the programmer specifically

```cpp
// LightEngine::Commands - the grammar-aware layer, only place NewSyntaxParser is visible
Utils::Commands::Console<Engine::Engine> console(engine, "prog> ");

console.commands().add(".help", "Show commands", "", {}, [](auto &, auto &) { /* ... */ return true; });
console.commands().add(".stored", "Dump pools", "", {},
    [](auto &, Engine::Engine &e) { std::cout << e.presets().toString(); return true; });

CommandParser parser(tokensFile, grammarFile);
CommandExecutor executor(engine);
console.onUnmatched([&](const std::string &line, Engine::Engine &) {
    auto program = parser.parse(line);
    if (program.empty()) return false;
    executor.run(program);
    return true;
});

console.run();
```

`Ctx = Engine&` here specifically (not `Programmer&`), because
`CommandExecutor` needs `Engine` for the store/recall coordination — the
"programmer console" and "programmer-only API" aren't the same scope once
presets enter the picture.

## Wiring it for patch — same template, different `Ctx`, no grammar at all

```cpp
Utils::Commands::Console<Components::Patch> patchConsole(engine.patcher(), "patch> ");
patchConsole.commands().add("patch", "Patch fixtures", "patch <template> <uni> <amount>", {},
    [&lib](const Args &a, Components::Patch &p) {
        auto *fx = lib.find(a[1]);
        if (!fx) return false;
        p.patch(fx, std::stoi(a[2]), std::stoi(a[3]));
        return true;
    });
patchConsole.commands().add("unpatch", "Remove a fixture", "unpatch <fid>", {},
    [](const Args &a, Components::Patch &p) { return p.unpatch(std::stoul(a[1])); });
// no onUnmatched() call - unmatched lines just report "unknown command"
patchConsole.run();
```

`Ctx = Patch&` directly — no `Engine` needed at all, because patch operations
genuinely are single-component (matches `Patch.h`'s own "knows nothing above
it" API rule). This is the concrete payoff of the template being generic over
`Ctx`: the patch console literally cannot reach `Programmer` or `Presets`, by
construction, not by convention.

## Open questions before implementing

1. **`Console`'s constructor takes `Ctx&` by reference** and holds it for the
   object's lifetime — fine for a REPL that outlives one session, but means
   one `Console` per `Ctx` instance.
2. **Where do the two consoles run** — sequentially (finish patching, `.quit`,
   then drop into the programmer console), or both live at once? `Terminal`
   takes over the whole TTY in raw mode, so two `Console`s can't both `run()`
   concurrently on the same terminal without more plumbing (a mode-switch
   key, or separate ttys/panes). Patching being a setup-time concern suggests
   sequential is right, but this affects whether `Console::run()` needs an
   early-return "switch mode" signal.
