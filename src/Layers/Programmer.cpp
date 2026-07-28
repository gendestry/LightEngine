#include "LightEngine/Engine/Layers/Programmer.h"

#include "Utils/Colors/Colors.h"

namespace LightEngine::Engine
{
DMX::FixtureGroup Programmer::singleFixture(uint16_t fid) const
{
    DMX::FixtureGroup g;
    g.add(m_patch.getFixtures({fid}));
    return g;
}

namespace
{
// Force value to 1 so the hue/sat survives the RGB round-trip a ColorEffect
// does internally (a value of 0 would collapse to black and lose the hue).
Utils::Colors::RGB hueSatToRgb(float h, float s)
{
    return Utils::Colors::hsvToRgb(Utils::Colors::HSV{h, s, 1.f});
}
} // namespace

// void Programmer::setColor(const Utils::Colors::HSV &hsv)
// {
//     // Colour and intensity are independent: a ColorEffect only writes
//     push(std::make_unique<Effects::ColorEffect>(m_selection,
//                                                 hueSatToRgb(hsv.h, hsv.s)));
// }

void Programmer::setColor(const Utils::Colors::RGB &rgb)
{
    push(std::make_unique<Effects::StaticColor>(m_selection, rgb));
}

// void Programmer::setHueSat(float h, float s)
// {
//     push(
//         std::make_unique<Effects::ColorEffect>(m_selection, hueSatToRgb(h,
//         s)));
// }

void Programmer::setIntensity(float v)
{
    push(std::make_unique<Effects::StaticIntensity>(m_selection, v));
}

// void Programmer::setIntensityRamp(float a, float b)
// {
//     // Per-fixture levels -> one constant DimmerEffect per fixture.
//     const auto &fixtures = m_selection.fixtures();
//     const std::size_t n = fixtures.size();
//     for (std::size_t i = 0; i < n; ++i)
//     {
//         float t = n <= 1 ? 0.f : float(i) / float(n - 1);
//         push(std::make_unique<Effects::DimmerEffect>(
//             singleFixture(fixtures[i]->Fid()), a + (b - a) * t));
//     }
// }

// void Programmer::fanColor(float hueA, float hueB, float sat)
// {
//     const auto &fixtures = m_selection.fixtures();
//     const std::size_t n = fixtures.size();
//     for (std::size_t i = 0; i < n; ++i)
//     {
//         float t = n <= 1 ? 0.f : float(i) / float(n - 1);
//         push(std::make_unique<Effects::ColorEffect>(
//             singleFixture(fixtures[i]->Fid()),
//             hueSatToRgb(hueA + (hueB - hueA) * t, sat)));
//     }
// }

// void Programmer::applyHueSat(uint16_t fid, float h, float s)
// {
//     push(std::make_unique<Effects::ColorEffect>(singleFixture(fid),
//                                                 hueSatToRgb(h, s)));
// }

// void Programmer::applyIntensity(uint16_t fid, float v)
// {
//     push(std::make_unique<Effects::DimmerEffect>(singleFixture(fid), v));
// }

// std::map<uint16_t, FixtureValues> Programmer::edits() const
// {
//     // Compose the stack the way a frame does; constant effects ignore time.
//     Frame frame;
//     const TimeContext t;
//     for (const auto &e : m_effects)
//     {
//         e->apply(frame, t);
//     }
//     return frame.all();
// }

void Programmer::apply(Frame &frame, const TimeContext &time)
{
    // The programmer is just a layer of effects, replayed every frame ->
    for (const auto &e : m_effects)
    {
        if (e->enabled())
        {
            e->apply(frame, time);
        }
    }
}
} // namespace LightEngine::Engine
