// #include "LightEngine/Engine/Layer.h"

// #include "Utils/Colors/Colors.h"

// namespace LightEngine::Engine
// {
// DMX::FixtureGroup ProgrammerLayer::singleFixture(uint16_t fid) const
// {
//     DMX::FixtureGroup g;
//     g.add(m_patch.getFixtures({fid}));
//     return g;
// }

// namespace
// {
// // Force value to 1 so the hue/sat survives the RGB round-trip a ColorEffect
// // does internally (a value of 0 would collapse to black and lose the hue).
// Utils::Colors::RGB hueSatToRgb(float h, float s)
// {
//     return Utils::Colors::hsvToRgb(Utils::Colors::HSV{h, s, 1.f});
// }
// } // namespace

// void ProgrammerLayer::setColor(const Utils::Colors::HSV &hsv)
// {
//     // Colour and intensity are independent: a ColorEffect only writes
//     hue/sat. push(std::make_unique<Effects::ColorEffect>(m_selection,
//                                                 hueSatToRgb(hsv.h, hsv.s)));
// }

// void ProgrammerLayer::setColor(const Utils::Colors::RGB &rgb)
// {
//     push(std::make_unique<Effects::ColorEffect>(m_selection, rgb));
// }

// void ProgrammerLayer::setHueSat(float h, float s)
// {
//     push(std::make_unique<Effects::ColorEffect>(m_selection, hueSatToRgb(h,
//     s)));
// }

// void ProgrammerLayer::setIntensity(float v)
// {
//     push(std::make_unique<Effects::DimmerEffect>(m_selection, v));
// }

// void ProgrammerLayer::setIntensityRamp(float a, float b)
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

// void ProgrammerLayer::fanColor(float hueA, float hueB, float sat)
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

// void ProgrammerLayer::applyHueSat(uint16_t fid, float h, float s)
// {
//     push(std::make_unique<Effects::ColorEffect>(singleFixture(fid),
//                                                 hueSatToRgb(h, s)));
// }

// void ProgrammerLayer::applyIntensity(uint16_t fid, float v)
// {
//     push(std::make_unique<Effects::DimmerEffect>(singleFixture(fid), v));
// }

// std::map<uint16_t, FixtureValues> ProgrammerLayer::edits() const
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

// void ProgrammerLayer::apply(Frame &frame, const TimeContext &time)
// {
//     // The programmer is just a layer of effects, replayed every frame ->
//     live. for (const auto &e : m_effects)
//     {
//         if (e->enabled())
//         {
//             e->apply(frame, time);
//         }
//     }
// }
// } // namespace LightEngine::Engine
