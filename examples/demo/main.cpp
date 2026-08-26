#include <iostream>
#include <string>

#include "LightEngine/Engine/Engine.h"
#include "Utils/Colors/HSV.h"

using namespace LightEngine;

int main()
{
    Utils::Logger logger("test");
    logger.setLevel(Utils::Logger::DEBUGGING);

    Engine::Engine engine;

    auto &lib = engine.fixtureLibrary();
    auto rgb = lib.find("RGB");

    auto &patch = engine.patcher();

    auto f1 = patch.patch(rgb, 1, 10, 0, 101);
    auto f2 = patch.patch(rgb, 2, 10);

    auto &prog = engine.programmer();
    // prog.select(f1);
    // prog.setColor({255, 128, 0});
    // prog.addDimmerChase();
    // engine.storeGroup();

    // prog.select(f2);
    // prog.setColor({255, 0, 207});
    // prog.setIntensity(0.8f);
    // engine.storeGroup();

    prog.select({1, 2, 3, 4});
    prog.add({8, 10, 13, 14, 15});

    // int i = 0;
    // for (auto [k, fix] : engine.patcher().fixs())
    // {
    //     Engine::FixtureValues v;
    //     v.intensity = i++ / (static_cast<float>(20.f) - 1.f);
    //     v.color = Utils::Colors::RGB(255, 128, 0).toHSV();
    //     fix->Resolve(v);
    // }
    engine.update();
    engine.patcher().print();
    engine.patcher().removeFixture(2);
    engine.patcher().unpatch(4);
    engine.patcher().unpatch(5);
    engine.patcher().unpatch(6);
    engine.patcher().patch(6, 2, 200);

    engine.patcher().print();

    // std::cout << engine.patcher().toString();

    return 0;
    /*
        // RGB fixtures (3 channels each) across three universes.
        auto fids8 = engine.patch(rgb, 8, 93);
        auto fids9 = engine.patch(rgb, 9, 120);
        auto fids10 = engine.patch(rgb, 10, 60);
        // auto fids8 = engine.patch(rgb, 1, 10);
        // auto fids9 = engine.patch(rgb, 1, 10);
        // auto fids10 = engine.patch(rgb, 1, 10);
        // auto fids10 = engine.patch(rgb, 10, 11);

        auto &prog = engine.programmer();
        prog.select(fids8);
        prog.setIntensity(1.f);

        prog.select(fids9);
        prog.setIntensity(0.5f);
        // prog.add(fids9);
        // prog.add(fids10);
        // engine.storeGroup();
        // prog.select(fids8);
        // engine.storeGroup();
        // prog.select(fids9);
        // engine.storeGroup();
        // prog.select(fids10);
        // engine.storeGroup();
        // // prog.select(fids8);
        // // engine.selectGroup(2);
        // // prog.setColor({255, 128, 0});
        // engine.selectGroup(2);
        // // prog.select(fids9);
        // prog.setColor({0, 0, 255});
        // engine.selectGroup(3);
        // prog.setColor({255, 0, 0});
        // engine.appendGroup(4);
        // prog.setIntensity(1.f);
        // engine.selectGroup(2);
        // engine.appendGroup(3);
        // engine.storeColorPreset(1);

        // // prog.add(fids8);
        // // engine.storeColorPreset();
        // prog.clearAll();
        // engine.selectGroup(1);
        // engine.recallColorPreset(1);
        // prog.setIntensity(1.f);
        // auto colors = prog.getStaticEffects().getColor(prog.selected());
        // for (auto &[k, v] : colors)
        // {
        //     std::cout << std::format("{}, {}\n", k, v.color->toString());
        // }
        // engine.recallColorPreset(1);
        // prog.setIntensity(0.7f);
        // std::cout << prog.toString();

        std::cout << engine.presets().toString() << std::endl;

        // prog.select(fids);
        // prog.setColor({255, 128, 0});

        auto &h = prog.getStaticEffects().getValues();
        for (auto &[k, v] : h)
        {
            // const auto vals = h;
            engine.patcher().getFixture(k)->Resolve(v);
        }

        engine.update();
        std::cout << engine.toString();
        return 0;
        // for()
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
        // const float dt = 1.f / 40.f;
        // const std::size_t frames = 80;

        // std::cout << "frame   t      fid1..fid" << fids8.size() << " (R channel)\n";
        // for (std::size_t i = 0; i < frames; ++i)
        // {
        //     engine.update(dt);

        //     // std::cout << std::setw(5) << i << std::setw(7) << std::fixed
        //     //           << std::setprecision(3) << engine.time().now << "   ";
        //     for (uint16_t fid : fids8)
        //     {
        //         // R is the fixture's first channel; read it straight out of the
        //         // // universe buffer, i.e. the bytes that would go on the wire.
        //         // const auto fx = engine.patcher().getFixture(fid);
        //         // const auto &buf =
        //         //     engine.patcher().universes().at(fx->Universe()).buffer();
        //         // std::cout << std::setw(4) << static_cast<int>(buf[fx->start]);
        //     }
        //     // std::cout << "\n";
        // }

        // std::cout << "\nfinal universe state:\n";
        // for (const auto &[id, uni] : engine.patcher().universes())
        // {
        //     std::cout << "Universe " << id << ":\n"
        //               << uni.dump() << "\n";
        // }

        // std::cout << engine.toString();
        // return 0;
        // */
};
