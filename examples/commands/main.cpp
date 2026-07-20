#include <algorithm>
#include <iostream>
#include <string>

#include "LightEngine/Engine/Engine.h"
#include "LightEngine/Engine/FixtureBuilder.h"
#include "Utils/Colors/HSV.h"
#include "Utils/Network/Interfaces.h"

using namespace LightEngine;
using Fixtures::Fixture;
using GDTF::Attribute;

// Absolute data dir baked in by CMake; falls back to a repo-root-relative path
// if built some other way.
#ifndef LE_DATA_DIR
#define LE_DATA_DIR "include/LightEngine/Commands/data"
#endif

// Run the command pipeline against a simple patch:
//   text -> CommandParser -> AST -> CommandExecutor -> Engine
int main()
{
    Engine::Engine engine;
    const Utils::Network::IP ip = Utils::Network::Interfaces::primaryIP();
    engine.setSourceName("LightEngine");
    engine.setIP(ip);

    // Same rig as the demo: a 3-channel RGB fixture across three universes.
    Fixture rgb =
        Engine::FixtureBuilder(
            "RGB", {Attribute::COLOR_R, Attribute::COLOR_G, Attribute::COLOR_B})
            .Get();
    engine.patch(rgb, 8, 93);
    engine.patch(rgb, 9, 120);
    engine.patch(rgb, 10, 60); // fids 1..273

    // Load the grammar + token definitions (LE_DATA_DIR is an absolute path
    // baked in at compile time, so this works from any working directory).
    engine.loadCommands(LE_DATA_DIR "/commands.txt",
                        LE_DATA_DIR "/commands.syn");

    auto &prog = engine.programmer();

    // Interactive REPL: read a command line, parse + execute it, report state.
    // Meta-commands (leading '.') aren't part of the grammar - they're handled
    // here for inspection/quit.
    // Meta-command help text (also printed on startup and via '.help').
    const char *help =
        "LightEngine command console (fids 1..273)\n"
        "  grammar commands:\n"
        "    <n> [thru <n>] [+/- ...]   select fixtures      e.g. 1 thru 4 + 8 "
        "- 2\n"
        "    group <n>                  select a stored group\n"
        "    at <level>                 set intensity 0..100 on the selection\n"
        "    store group <n>            bank the selection as a group\n"
        "    delete group <n>           remove a group\n"
        "    clear                      wipe the programmer\n"
        "  meta commands (not part of the grammar):\n"
        "    .help            show this help\n"
        "    .stored          dump the pools\n"
        "    .printuniverses  render + dump each universe's DMX buffer\n"
        "    .quit            exit (or Ctrl+D)\n";

    std::cout << help;

    std::string line;
    while (true)
    {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) // EOF (Ctrl+D) ends the loop
            break;
        if (line.empty())
            continue;

        if (line == ".quit" || line == ".q")
            break;
        if (line == ".help" || line == ".h")
        {
            std::cout << help;
            continue;
        }
        if (line == ".stored")
        {
            std::cout << engine.stored().describe() << "\n";
            continue;
        }
        if (line == ".printuniverses" || line == ".u")
        {
            // Resolve the current programmer state into the DMX buffers so the
            // dump reflects what would go on the wire.
            engine.update();
            const auto &universes = engine.patcher().universes();
            if (universes.empty())
                std::cout << "  (no universes patched)\n";
            for (const auto &[id, uni] : universes)
            {
                // Cover every patched channel in this universe (packed dense).
                std::size_t used = 0;
                for (const auto &f : uni.fixtures())
                    used += f->Footprint();
                const int channels =
                    static_cast<int>(std::clamp<std::size_t>(used, 16, 512));
                // dump() already prints a "Universe N (M fixtures)" header.
                std::cout << uni.dump(channels) << "\n";
            }
            continue;
        }

        if (!engine.command(line))
        {
            std::cout << "  parse error\n";
            continue;
        }
        // Tick the engine so the command's edits compose -> resolve -> output
        // immediately: the programmer is a live layer, so every command lands
        // on the wire right away.
        engine.update();
        std::cout << "  ok [selection=" << prog.selection().size()
                  << " edits=" << prog.edits().size() << "]\n";
    }

    std::cout << "bye\n";
    return 0;
}
