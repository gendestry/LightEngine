#include "LightEngine/Engine/Layers/Programmer.h"

#include "LightEngine/Effects/Static/EffectColor.h"
#include "LightEngine/Effects/Static/EffectIntensity.h"
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

void Programmer::select(const DMX::FixtureGroup &g) { m_selection.select(g); }

void Programmer::select(const std::vector<uint16_t> &fids)
{
    DMX::FixtureGroup g(m_patch.getFixtures(fids));
    m_selection.select(std::move(g));
}

void Programmer::add(const DMX::FixtureGroup &g) { m_selection.add(g); }

void Programmer::add(const std::vector<uint16_t> &fids)
{
    m_selection.add(m_patch.getFixtures(fids));
}

// void Programmer::applyEffect(std::shared_ptr<Effects::EffectWrapper> eff)
// {
//     auto g = eff->group & m_selected;
//     m_runningEffects.push(eff);
// }

void Programmer::setColor(const Utils::Colors::RGB &rgb)
{
    auto &group = m_selection.get();
    m_staticEffects.setColor(group, rgb);
    m_runningEffects.pushRet<Effects::StaticColor>(
        group, m_staticEffects.getColor(group));
}

// void Programmer::setColorGradient(const Utils::Colors::RGB &rgb,
//                                   const Utils::Colors::RGB &rgb2)
// {
//     running.push<Effects::StaticColorGrad>(rgb, rgb2);
// }

// void Programmer::setHueSat(float h, float s)
// {
//     push(
//         std::make_unique<Effects::ColorEffect>(m_selection, hueSatToRgb(h,
//         s)));
// }

void Programmer::setIntensity(float v)
{
    auto &group = m_selection.get();
    m_staticEffects.setIntensity(group, v);
    m_runningEffects.pushRet<Effects::StaticIntensity>(
        group, m_staticEffects.getIntensity(group));
    // m_staticEffects[Effects::StaticIntensity::GetStaticCategory()].push_back(
    //     m_runningEffects.pushRet<Effects::StaticIntensity>(m_selected, v));
    // running.push<Effects::StaticIntensity>(v);
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

// void Programmer::apply(Frame &frame, const TimeContext &time)
// {
//     // The programmer is just a layer of effects, replayed every frame ->
//     walk
//     // every stack in order, then every effect within it. An effect only
//     // recomputes when it is due (edited, its selection changed, or its own
//     beat
//     // grid says a new step arrived); otherwise its cached output is
//     replayed. for (auto &eff : m_runningEffects.getEffects())
//     {
//         const auto &group = eff->group;
//         const auto &e = eff->effect;
//         if (!e->enabled())
//         {
//             continue;
//         }
//         if (e->due(time.now, group))
//         {
//             e->evaluate(time, group);
//         }
//         e->replay(frame);
//     }

//     // for (auto &stack : running.stacks)
//     // {
//     //     const auto &group = stack.selection;
//     //     for (const auto &e : stack.effects)
//     //     {
//     //         if (!e->enabled())
//     //         {
//     //             continue;
//     //         }
//     //         if (e->due(time.now, group))
//     //         {
//     //             e->evaluate(time, group);
//     //         }
//     //         e->replay(frame);
//     //     }
//     // }
// }
} // namespace LightEngine::Engine
