// Full-functionality walkthrough of the engine's programmer / effects /
// presets pipeline:
//   patch -> select -> program (static + animated effects) -> update
//         -> store (group, color preset, dimmer preset) -> recall
//
// Mirrors the console session used to design the store/recall feature:
//   group1 = fid 1-10, group2 = fid 11-20, group3 = fid 21-30, group4 = 1-30
//   select group1+2; at 100%
//   select group2;   color orange
//   select fix 8-15; color blue        (overlaps group1/group2 - LTP wins)
//   select group3;   dimmer chase      (animated - excluded from presets)
//   select group4;   store preset color 1  -> 8-15 blue, 16-20 orange
//   select group1;   store preset color 2  -> only 8,9,10 (blue)
//   select group4;   store preset dimmer 1 -> only 1-20 @ 100% (chase excluded)
//   select 1-10;     recall preset color 1 -> only 8,9,10 change
#include <iostream>
#include <string>

#include "LightEngine/Engine/Engine.h"
#include "Utils/Colors/Colors.h"

using namespace LightEngine;
using Utils::Maths::Interval;

#ifndef DEFAULT_DATA_DIR
#define DEFAULT_DATA_DIR "include/LightEngine/Commands/Default/data"
#endif

namespace
{
Interval range(uint64_t from, uint64_t to)
{
    Interval i;
    i.add(from, to);
    return i;
}

void heading(const std::string &title)
{
    std::cout << "\n=== " << title << " ===\n";
}
} // namespace

int main()
{
    Utils::Logger logger("presets-demo");
    logger.setLevel(Utils::Logger::DEBUGGING);

    Engine::Engine engine;
    engine.loadCommands(DEFAULT_DATA_DIR "/default.tok", DEFAULT_DATA_DIR "/default.syn");

    auto &lib = engine.fixtureLibrary();
    auto rgb = lib.find("RGB");
    auto &patch = engine.patcher();
    patch.patch(rgb, /*universe*/ 8, /*amount*/ 93);
    patch.patch(rgb, /*universe*/ 9, /*amount*/ 120);
    patch.patch(rgb, /*universe*/ 10, /*amount*/ 60);

    std::string line;
    while (std::getline(std::cin, line))
    {
        if (!engine.command(line))
        {
            std::cout << "  parse error\n";
            continue;
        }
        engine.update();
    }
    // ---- patch --------------------------------------------------------
    // 30 RGB fixtures on universe 1, FIDs 1-30, 3 DMX channels apart.
    // heading("Patch");

    // std::cout << "Patched 30 RGB fixtures on universe 1 (fid 1-30)\n";

    const Interval group1 = range(1, 10);
    const Interval group2 = range(11, 20);
    const Interval group3 = range(21, 30);
    Interval group4 = group1;
    group4 |= group2;
    group4 |= group3;

    // ---- program: static edits -----------------------------------------
    // heading("Program");
    auto &prog = engine.programmer();

    prog.select(group1);
    prog.add(group2);
    prog.setIntensity(1.0f);
    auto &x = engine.storeDimmerPreset();
    x.setName("100");

    prog.select(group2);
    prog.setColor({255, 128, 0}); // orange
    engine.storeColorPreset();

    prog.select(range(8, 15));
    prog.setColor({0, 100, 255}); // blue - overlaps group2 on 11-15, LTP wins
    engine.storeColorPreset();
    logger.info(engine.presets().toString());

    prog.clearAll();
    prog.select(group1 | group2);
    engine.recallDimmerPreset(1);
    engine.recallColorPreset(2);
    engine.update();
    logger.info(patch.uniDumpStr());


    // ---- program: animated effect --------------------------------------
//     prog.select(group3);
//     prog.addDimmerChase();
//
//     // ---- update: run a few frames so the chase actually animates -------
//     heading("Update");
//     for (int i = 0; i < 5; ++i)
//     {
//         engine.update(1.f / 40.f); // 40 fps
//     }
//     std::cout << "Ran 5 frames at 40fps (t = " << engine.time().now << "s)\n";
//     std::cout << "Universe 1 (first 30 channels):\n"
//               << patch.uniDumpStr();
//
//     // ---- store: a fixture group ------------------------------------------
//     heading("Store - group");
//     prog.select(group4);
//     auto &storedGroup = engine.storeGroup(1);
//     std::cout << storedGroup.toString() << "\n";
//
//     // ---- store: presets, scoped to whatever is selected -----------------
//     // Static-only, and masked by the selection at store time: a preset never
//     // captures fixtures owned by a running animated effect in that category.
//     heading("Store - presets");
//
//     prog.select(group4); // 1-30
//     auto &colorPreset1 = engine.storeColorPreset(1);
//     std::cout << "store preset color 1 (group4 selected): "
//               << colorPreset1.effect()->values().size() << " fixtures captured\n"
//               << "  expected: 8-15 blue, 16-20 orange (21-30 excluded - chase-owned)\n";
//
//     prog.select(group1); // 1-10
//     auto &colorPreset2 = engine.storeColorPreset(2);
//     std::cout << "store preset color 2 (group1 selected): "
//               << colorPreset2.effect()->values().size() << " fixtures captured\n"
//               << "  expected: only 8,9,10 (only overlap between group1 and the blue set)\n";
//
//     prog.select(group4); // 1-30
//     auto &dimmerPreset1 = engine.storeDimmerPreset(1);
//     std::cout << "store preset dimmer 1 (group4 selected): "
//               << dimmerPreset1.effect()->values().size() << " fixtures captured\n"
//               << "  expected: only 1-20 @ 100% (21-30 excluded - chase-owned)\n";
//
//     std::cout << "\n" << engine.presets().toString() << "\n";
//
//     // ---- recall: masked by whatever is selected NOW, not by what was ---
//     // originally stored. Recalling color preset 1 (which covers 8-20) onto
//     // just 1-10 only touches the overlap: 8, 9, 10.
//     heading("Recall");
//     prog.clearAll();
//     prog.select(group1); // 1-10
//     engine.recallColorPreset(1);
//     engine.update(1.f / 40.f);
//     std::cout << "clearAll(); select group1 (1-10); recall preset color 1\n"
//               << "  expected: only fid 8,9,10 turn blue - 1-7 stay unlit, 11-30 untouched\n\n"
//               << "Universe 1 after recall (first 30 channels):\n"
//               << patch.uniDumpStr();
//
     return 0;
}
