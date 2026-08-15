// #include <atomic>
// #include <chrono>
// #include <iostream>
// #include <mutex>
// #include <string>
// #include <thread>

// // #include "LightEngine/Commands/CommandExecutor.h"
// // #include "LightEngine/Commands/CommandParser.h"
// // #include "LightEngine/DMX/Universe.h"
// #include "LightEngine/Engine/Engine.h"
// #include "LightEngine/Engine/FixtureBuilder.h"
// #include "Utils/Colors/HSV.h"
// #include "Utils/Commands/Args.h"
// #include "Utils/Network/Interfaces.h"

// using namespace LightEngine;
// using Fixtures::Fixture;
// using GDTF::Attribute;

// // Absolute data dir baked in by CMake; falls back to a repo-root-relative
// path
// // if built some other way.
// #ifndef LE_DATA_DIR
// #define LE_DATA_DIR "include/LightEngine/Commands/data"
// #endif

// int main()
// {
//     Engine::Engine engine;

//     // a simple 3-channel RGB fixture, built with the 8-bit FixtureBuilder
//     Fixture rgb =
//         Engine::FixtureBuilder(
//             "RGB", {Attribute::COLOR_R, Attribute::COLOR_G,
//             Attribute::COLOR_B}) .Get();

//     // RGB fixtures (3 channels each) across three universes.
//     auto fids8 = engine.patch(rgb, 8, 93);
//     auto fids9 = engine.patch(rgb, 9, 120);
//     auto fids10 = engine.patch(rgb, 10, 60);

//     // build a selection from everything we just patched, then store it as
//     // group 1: select() starts fresh, add() accumulates (ordered, deduped).
//     auto &prog = engine.programmer();
//     prog.select(fids8);
//     prog.add(fids9);
//     prog.add(fids10);
//     engine.storeGroup(1);
//     engine.stored().groups().rename(1, "all");
//     // prog.setIntensity(0.5f);
//     prog.clearAll();
//     prog.select(fids8);
//     engine.storeGroup(2);
//     prog.clearAll();

//     engine.selectGroup(1);
//     prog.setIntensity(1.f);
//     prog.setColor(Utils::Colors::RGB{0, 0, 255});

//     // // fan hue red->blue across the selection, full intensity, then bank
//     it
//     // as
//     // // color preset 1 and dimmer preset 1.
//     // prog.fanColor(0.f, 240.f); // hue 0 (red) .. 240 (blue)
//     // prog.setIntensityRamp(0.25f, 1.f);
//     // engine.storeColorPreset(1);
//     // engine.stored().colorPresets().rename(1, "Rainbow");
//     // engine.storeDimmerPreset(1);
//     // engine.stored().dimmerPresets().rename(1, "Ramp");

//     // // ---- functional test
//     // ---------------------------------------------------- auto probe =
//     // [&](const char *label)
//     // {
//     //     engine.update();
//     //     const auto *v = engine.programmer().edits().empty()
//     //                         ? nullptr
//     //                         :
//     &engine.programmer().edits().begin()->second;
//     //     std::cout << label << ": edits=" <<
//     //     engine.programmer().edits().size(); if (v && v->color)
//     //         std::cout << " fid" <<
//     engine.programmer().edits().begin()->first
//     //                   << " h=" << v->color->h << " s=" << v->color->s
//     //                   << " v=" << v->color->v;
//     //     std::cout << "\n";
//     // };

//     // probe("after fan+ramp");

//     // // clear, then recall both presets onto a fresh selection of the group
//     // engine.clear();
//     // probe("after clear"); // expect edits=0

//     // engine.selectGroup(1);
//     // engine.recallColorPreset(1);
//     // engine.recallDimmerPreset(1);
//     // probe("after recall color+dimmer"); // expect edits back, h/s/v set

//     // std::cout << engine.stored().describe() << "\n";

//     // // ---- command executor test (hand-built AST)
//     // -----------------------------
//     // // Equivalent command line:  clear ; 1 thru 6 at 100 ; store group 2
//     // {
//     //     using namespace Macros;
//     //     Program cmds;

//     //     auto clr = std::make_unique<ClearCmd>();
//     //     cmds.push_back(std::move(clr));

//     //     auto sc = std::make_unique<SelectCmd>();
//     //     Item item;
//     //     item.op = "";
//     //     auto range = std::make_unique<FixtureRange>();
//     //     range->from = 1;
//     //     range->to = 6;
//     //     item.sel = std::move(range);
//     //     sc->items.push_back(std::move(item));
//     //     sc->hasAt = true;
//     //     sc->at.kind = "level";
//     //     sc->at.level = 100;
//     //     cmds.push_back(std::move(sc));

//     //     auto store = std::make_unique<StoreCmd>();
//     //     auto grp = std::make_unique<Group>();
//     //     grp->id = 2;
//     //     store->target = std::move(grp);
//     //     cmds.push_back(std::move(store));

//     //     Commands::CommandExecutor exec(engine);
//     //     exec.run(cmds);

//     //     std::cout << "[cmd] selection="
//     //               << engine.programmer().selection().size()
//     //               << " edits=" << engine.programmer().edits().size();
//     //     if (auto g2 = engine.stored().groups().get(2))
//     //         std::cout << " group2.fids=" << g2->fids().size();
//     //     std::cout << "\n";
//     // }

//     // // ---- full pipeline: parse text -> AST -> execute
//     // ------------------------
//     // {
//     //     Commands::CommandParser parser(LE_DATA_DIR "/commands.txt",
//     //                                    LE_DATA_DIR "/commands.syn");
//     //     Commands::CommandExecutor exec(engine);

//     //     for (const char *line :
//     //          {"clear", "1 thru 4 + 7 - 2", "at 50", "store group 3"})
//     //     {
//     //         auto prog = parser.parse(line);
//     //         exec.run(prog);
//     //         std::cout << "[parse] \"" << line << "\" -> cmds=" <<
//     prog.size()
//     //                   << " selection=" <<
//     //                   engine.programmer().selection().size()
//     //                   << "\n";
//     //     }
//     //     if (auto g3 = engine.stored().groups().get(3))
//     //         std::cout << "[parse] group3.fids=" << g3->fids().size() <<
//     "\n";
//     // }

//     // // ---- accumulate across selections: a subset + a stray fixture
//     // ----------
//     // {
//     //     engine.clear();
//     //     engine.programmer().select({1, 2, 3}); // first selection
//     //     prog.fanColor(0.f, 240.f);
//     //     engine.programmer().select({50}); // replaces selection; edits
//     //     persist prog.setColor(Utils::Colors::HSV(300.f, 1.f, 1.f));
//     //     engine.storeColorPreset(5); // should hold 3 + 1 = 4

//     //     std::cout << "[accum] selection="
//     //               << engine.programmer().selection().size() << "
//     //               preset5.size="
//     //               << engine.stored().colorPresets().get(5)->size() <<
//     "\n";
//     // }

//     // output: stream sACN from this machine's primary interface
//     const Utils::Network::IP ip = Utils::Network::Interfaces::primaryIP();
//     engine.setSourceName("LightEngine");
//     engine.setIP(ip);

//     // render one frame and show the resulting DMX for universe 8
//     // engine.update();
//     if (DMX::Universe *u = engine.getUniverse(8))
//         std::cout << u->dump() << "\n";

//     std::cout << "Streaming sACN from " << ip.str()
//               << " (universes 8, 9, 10) - Ctrl+C to stop\n";

//     // every fixture we patched, for the per-frame color print
//     std::vector<uint16_t> allFids;
//     allFids.insert(allFids.end(), fids8.begin(), fids8.end());
//     allFids.insert(allFids.end(), fids9.begin(), fids9.end());
//     allFids.insert(allFids.end(), fids10.begin(), fids10.end());

//     // The engine is driven from a background thread; the main thread reads
//     // commands from stdin. Both touch the engine, so guard it with a mutex.
//     std::mutex engineMutex;
//     std::atomic<bool> running{true};

//     // continuous full-frame output, like a real sACN source (~40 Hz)
//     using namespace std::chrono;
//     std::thread renderThread(
//         [&]()
//         {
//             const auto period = milliseconds(25);
//             auto last = steady_clock::now();
//             while (running.load())
//             {
//                 const auto now = steady_clock::now();
//                 const float dt = duration<float>(now - last).count();
//                 last = now;

//                 {
//                     std::lock_guard<std::mutex> lock(engineMutex);
//                     engine.update(dt); // compose -> resolve -> send
//                 }
//                 std::this_thread::sleep_for(period);
//             }
//         });

//     // input loop: "color <r> <g> <b>" sets the group color, "print" dumps
//     the
//     // resolved {R,G,B} of every fixture, "params" lists every fixture and
//     its
//     // parameters, "quit"/"exit" stops.
//     std::cout << "commands: color <r> <g> <b> | print | params | quit\n";
//     std::string line;
//     while (running.load() && std::getline(std::cin, line))
//     {
//         Utils::Commands::Args args(line);
//         if (args.empty())
//             continue;

//         const std::string &cmd = args.command();
//         std::lock_guard<std::mutex> lock(engineMutex);

//         if (cmd == "quit" || cmd == "exit")
//         {
//             running.store(false);
//         }
//         else if (cmd == "color" && args.getInt(1) && args.getInt(2) &&
//                  args.getInt(3))
//         {
//             prog.setColor(
//                 Utils::Colors::RGB{static_cast<uint8_t>(*args.getInt(1)),
//                                    static_cast<uint8_t>(*args.getInt(2)),
//                                    static_cast<uint8_t>(*args.getInt(3))});
//         }
//         else if (cmd == "print")
//         {
//             for (uint16_t f : allFids)
//             {
//                 const auto c = engine.color(f);
//                 std::cout << "fid" << f << ": {" << (int)c.r << "," <<
//                 (int)c.g
//                           << "," << (int)c.b << "}\n";
//             }
//         }
//         else if (cmd == "params")
//         {
//             for (const auto &[fid, fx] : engine.patcher().fixtures())
//             {
//                 std::cout << "fid" << fid << " \"" << fx->Name() << "\" u"
//                           << fx->Universe() << " @" << fx->start << " (+"
//                           << fx->size << " ch)\n";
//                 for (const Fixtures::Parameter &p : fx->Parameters())
//                 {
//                     const auto &ch = p.Definition()->channel;
//                     const bool bit16 =
//                         ch.res == GDTF::DMXChannel::Resolution::Bit16;
//                     std::cout << "    attr=" << (int)p.Attribute()
//                               << " cell=" << p.CellIndex()
//                               << " addr=" << (fx->start + ch.address)
//                               << (bit16 ? " (16-bit)" : " (8-bit)") << "
//                               value="
//                               << (int)engine.attributeValue(fid,
//                               p.Attribute(),
//                                                             p.CellIndex())
//                               << "\n";
//                 }
//             }
//         }
//         else
//         {
//             std::cout << "unknown command: " << cmd << "\n";
//         }
//     }

//     running.store(false);
//     renderThread.join();
//     return 0;
// }

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

// #include "LightEngine/Commands/CommandExecutor.h"
// #include "LightEngine/Commands/CommandParser.h"
// #include "LightEngine/DMX/Universe.h"
#include "LightEngine/Engine/Engine.h"
#include "LightEngine/Engine/FixtureBuilder.h"
#include "Utils/Colors/HSV.h"
// #include "Utils/Commands/Args.h"
#include "Utils/Network/Interfaces.h"

using namespace LightEngine;
using Fixtures::Fixture;
using GDTF::Attribute;

#ifndef LE_DATA_DIR
#define LE_DATA_DIR "include/LightEngine/Commands/data"
#endif

int main()
{
    Engine::Engine engine;

    auto &lib = engine.fixtureLibrary();
    auto rgb = lib.find("RGB").value();

    // RGB fixtures (3 channels each) across three universes.
    auto fids8 = engine.patch(rgb, 8, 10);
    // auto fids9 = engine.patch(rgb, 9, 10);
    // auto fids10 = engine.patch(rgb, 10, 11);

    auto &prog = engine.programmer();
    prog.select(fids8);
    // engine.storeGroup();
    // prog.clearAll();
    // prog.select(fids9);
    // engine.storeGroup();
    // prog.clearAll();
    // prog.select(fids10);
    // engine.storeGroup();
    // prog.clearAll();

    // for (auto &g : engine.stored().groups())
    // {
    //     for (auto &fid : g.second->fids())
    //     {
    //         std::cout << fid << std::endl;
    //     }

    //     std::cout << "New" << std::endl;
    // }

    // Red at full, with a dimmer chase riding on top. The chase merges HTP, so
    // it lifts levels rather than replacing them - no static intensity here, or
    // it would mask the wave entirely.
    // prog.setColor({255, 0, 0});
    // 8 samples per cycle at 120 BPM -> the chase has new output 16 times a
    // second. At 40 fps that is 16 recomputes instead of 40, and the frames in
    // between replay the cache.
    // prog.setIntensityRamp(Utils::Maths::TRIANGLE, 120.f, 1.f, 8);

    // Render at 40 fps for two seconds, printing fixture 1's R channel (colour
    // is red, so R == 255 * intensity) so the wave is visible as a column.
    const float dt = 1.f / 40.f;
    const std::size_t frames = 80;

    std::cout << "frame   t      fid1..fid" << fids8.size() << " (R channel)\n";
    for (std::size_t i = 0; i < frames; ++i)
    {
        engine.update(dt);

        std::cout << std::setw(5) << i << std::setw(7) << std::fixed
                  << std::setprecision(3) << engine.time().now << "   ";
        for (uint16_t fid : fids8)
        {
            // R is the fixture's first channel; read it straight out of the
            // universe buffer, i.e. the bytes that would go on the wire.
            const auto fx = engine.patcher().getFixture(fid);
            const auto &buf =
                engine.patcher().universes().at(fx->Universe()).buffer();
            std::cout << std::setw(4) << static_cast<int>(buf[fx->start]);
        }
        std::cout << "\n";
    }

    std::cout << "\nfinal universe state:\n";
    for (const auto &[id, uni] : engine.patcher().universes())
    {
        std::cout << "Universe " << id << ":\n" << uni.dump() << "\n";
    }

    std::cout << engine.describe();
    return 0;
};
