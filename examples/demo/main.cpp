#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/DMX/Universe.h"
#include "LightEngine/Engine/Engine.h"
#include "LightEngine/Fixture/Fixture.h"
#include "LightEngine/GDTF/LogicalChannel.h"
#include "Utils/Colors/Colors.h"
#include "Utils/Colors/Font.h"
#include "Utils/Colors/HSV.h"

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

    // 10 RGB fixtures (3 channels each) then 5 DimmerRGB (4 channels each),
    // all patched into universe 1.
    auto rgbFids = engine.patch(MakeRGB(), 1, 10);
    auto dimFids = engine.patch(MakeDimmerRGB(), 1, 5);

    // groups: 1 = the RGBs, 2 = the DimmerRGBs, 3 = all of them
    engine.addToGroup("rgb", rgbFids);
    engine.addToGroup("dimmer", dimFids);
    engine.addToGroup("all", rgbFids);
    engine.addToGroup("all", dimFids);

    // all editing goes through the programmer layer now (fixtures/groups hold
    // no state; the programmer does).
    auto &prog = engine.programmer();

    // colors: group 1 green, group 2 blue
    prog.select(*engine.getGroup("rgb"));
    prog.setColor(Utils::Colors::HSV(120.f, 1.f, 1.f));
    prog.select(*engine.getGroup("dimmer"));
    prog.setColor(Utils::Colors::HSV(240.f, 1.f, 1.f));

    // group 3: intensity ramp 0..1 across every fixture (V overrides the 1.0)
    prog.select(*engine.getGroup("all"));
    prog.setIntensityRamp(0.f, 1.f);

    // run one frame (blackout -> compose layers -> resolve), then print it
    engine.update();
    printUniverse(*engine.getUniverse(1), 64);
    return 0;
}
