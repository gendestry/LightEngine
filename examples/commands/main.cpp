#include <iostream>
#include <string>

#include "LightEngine/Engine/Engine.h"
#include "LightEngine/Engine/FixtureBuilder.h"
#include "Utils/Colors/HSV.h"

using namespace LightEngine;
using Fixtures::Fixture;
using GDTF::Attribute;

// Run the command pipeline against a simple patch:
//   text -> CommandParser -> AST -> CommandExecutor -> Engine
int main()
{
    Engine::Engine engine;

    // Same rig as the demo: a 3-channel RGB fixture across three universes.
    Fixture rgb = Engine::FixtureBuilder(
                      "RGB", {Attribute::COLOR_R, Attribute::COLOR_G,
                              Attribute::COLOR_B})
                      .Get();
    engine.patch(rgb, 8, 93);
    engine.patch(rgb, 9, 120);
    engine.patch(rgb, 10, 60); // fids 1..273

    // Load the grammar + token definitions (paths relative to the repo root,
    // the working directory when run as ./build/LightEngine_commands).
    engine.loadCommands("include/LightEngine/Commands/data/commands.txt",
                        "include/LightEngine/Commands/data/commands.syn");

    auto &prog = engine.programmer();

    // Interactive REPL: read a command line, parse + execute it, report state.
    // Meta-commands (leading '.') aren't part of the grammar - they're handled
    // here for inspection/quit.
    // Meta-command help text (also printed on startup and via '.help').
    const char *help =
        "LightEngine command console (fids 1..273)\n"
        "  grammar commands:\n"
        "    <n> [thru <n>] [+/- ...]   select fixtures      e.g. 1 thru 4 + 8 - 2\n"
        "    group <n>                  select a stored group\n"
        "    at <level>                 set intensity 0..100 on the selection\n"
        "    store group <n>            bank the selection as a group\n"
        "    delete group <n>           remove a group\n"
        "    clear                      wipe the programmer\n"
        "  meta commands (not part of the grammar):\n"
        "    .help     show this help\n"
        "    .stored   dump the pools\n"
        "    .quit     exit (or Ctrl+D)\n";

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

        if (!engine.command(line))
        {
            std::cout << "  parse error\n";
            continue;
        }
        std::cout << "  ok [selection=" << prog.selection().size()
                  << " edits=" << prog.edits().size() << "]\n";
    }

    std::cout << "bye\n";
    return 0;
}
