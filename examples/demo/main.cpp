#include <chrono>
#include <iostream>
#include <thread>

#include "LightEngine/DMX/Universe.h"
#include "LightEngine/Engine/Engine.h"
#include "LightEngine/Engine/FixtureBuilder.h"
#include "Utils/Colors/HSV.h"
#include "Utils/Network/Interfaces.h"

using namespace LightEngine;
using Fixtures::Fixture;
using GDTF::Attribute;

int main()
{
    Engine::Engine engine;

    // a simple 3-channel RGB fixture, built with the 8-bit FixtureBuilder
    Fixture rgb = Engine::FixtureBuilder(
                      "RGB", {Attribute::COLOR_R, Attribute::COLOR_G,
                              Attribute::COLOR_B})
                      .Get();

    // RGB fixtures (3 channels each) across three universes.
    auto fids8 = engine.patch(rgb, 8, 93);
    auto fids9 = engine.patch(rgb, 9, 120);
    auto fids10 = engine.patch(rgb, 10, 60);

    // one group holding everything we just patched
    engine.addToGroup("all", fids8);
    engine.addToGroup("all", fids9);
    engine.addToGroup("all", fids10);

    // set them all to yellow at full via the programmer layer
    auto &prog = engine.programmer();
    prog.select(*engine.getGroup("all"));
    prog.setColor(Utils::Colors::HSV(60.f, 1.f, 1.f)); // yellow, full intensity

    // output: stream sACN from this machine's primary interface
    const Utils::Network::IP ip = Utils::Network::Interfaces::primaryIP();
    engine.setSourceName("LightEngine");
    engine.setIP(ip);

    // render one frame and show the resulting DMX for universe 8
    engine.update();
    if (DMX::Universe *u = engine.getUniverse(8))
        std::cout << u->dump() << "\n";

    std::cout << "Streaming sACN from " << ip.str()
              << " (universes 8, 9, 10) - Ctrl+C to stop\n";

    // continuous full-frame output, like a real sACN source (~40 Hz)
    using namespace std::chrono;
    const auto period = milliseconds(25);
    auto last = steady_clock::now();
    while (true)
    {
        const auto now = steady_clock::now();
        const float dt = duration<float>(now - last).count();
        last = now;

        engine.update(dt); // compose -> resolve -> send
        std::this_thread::sleep_for(period);
    }
    return 0;
}
