#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/DMX/Universe.h"
#include "LightEngine/Engine/Engine.h"
#include "LightEngine/Fixture/Fixture.h"
#include "LightEngine/GDTF/LogicalChannel.h"
#include "Utils/Colors/Colors.h"
#include "Utils/Colors/Font.h"
#include "Utils/Colors/HSV.h"
#include "Utils/Network/Interfaces.h"

using namespace LightEngine;
using Fixtures::Fixture;
using GDTF::Attribute;

// pretty 16-column DMX dump: hex header, dashed separators, each channel padded
// to 3 and colored per owning fixture (unpatched channels dimmed).
static void printUniverse(DMX::Universe &universe, int channels = 64)
{
    namespace F = Utils::Font;

    // map every channel to the index of the fixture that owns it (-1 =
    // unpatched)
    const auto &fixtures = universe.fixtures();
    std::vector<int> owner(512, -1);
    int idx = 0;
    for (const auto &f : fixtures)
    {
        for (uint32_t c = 0; c < f->Footprint(); ++c)
        {
            owner[f->start + c] = idx;
        }
        ++idx;
    }

    auto colorFor = [](int ownerIdx) -> std::string
    {
        if (ownerIdx < 0)
        {
            return F::colorDim;
        }
        // distinct hue per fixture (same spirit as the old print)
        Utils::Colors::HSV hsv(std::fmod(110.f + ownerIdx * 65.f, 360.f), 0.5f,
                               0.9f);
        return F::colorByRGB(Utils::Colors::hsvToRgb(hsv), true);
    };

    const auto &buffer = universe.buffer();
    const int cols = 16;

    std::ostringstream ss;
    ss << "Universe " << universe.id() << "  (" << fixtures.size()
       << " fixtures)\n"
       << F::colorItalic;
    for (int i = 0; i < cols; ++i)
    {
        ss << "0x" << "0123456789ABCDEF"[i] << " ";
    }
    ss << F::colorReset << "\n";

    auto sep = [&]
    {
        for (int i = 0; i < cols; ++i)
            ss << "----";
        ss << "\n";
    };
    sep();

    for (int i = 0; i < channels; ++i)
    {
        if (i % cols == 0 && i != 0)
        {
            ss << "\n";
        }
        ss << colorFor(owner[i]) << std::setw(3) << int(buffer[i])
           << F::colorReset << " ";
    }
    ss << "\n";
    sep();
    std::cout << ss.str();
}

// one linear 8-bit channel: DMX 0..255 <-> physical 0..1
static std::shared_ptr<const GDTF::LogicalChannel> MakeChannel(Attribute attr,
                                                               uint16_t offset)
{
    GDTF::LogicalChannel lc;
    lc.attribute = attr;
    lc.channel = {offset, GDTF::DMXChannel::Resolution::Bit8};
    lc.functions = {{"", {0, 255}, {0.f, 1.f}}};
    return std::make_shared<const GDTF::LogicalChannel>(std::move(lc));
}

static Fixture MakeRGB()
{
    Fixture f("RGB");
    f.Add(MakeChannel(Attribute::COLOR_R, 0));
    f.Add(MakeChannel(Attribute::COLOR_G, 1));
    f.Add(MakeChannel(Attribute::COLOR_B, 2));
    f.Build();
    return f;
}

static Fixture MakeDimmerRGB()
{
    Fixture f("DimmerRGB");
    f.Add(MakeChannel(Attribute::DIMMER, 0));
    f.Add(MakeChannel(Attribute::COLOR_R, 1));
    f.Add(MakeChannel(Attribute::COLOR_G, 2));
    f.Add(MakeChannel(Attribute::COLOR_B, 3));
    f.Build();
    return f;
}

int main()
{
    Engine::Engine engine;

    // RGB fixtures (3 channels each) across three universes.
    Fixture rgb = MakeRGB();
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
