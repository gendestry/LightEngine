#include "LightEngine/Engine/Components/Programmer.h"

#include "LightEngine/Effects/Static/EffectColor.h"
#include "LightEngine/Effects/Static/EffectIntensity.h"
#include "Utils/Colors/Colors.h"

namespace LightEngine::Engine::Components
{
namespace
{
// Force value to 1 so the hue/sat survives the RGB round-trip a ColorEffect
// does internally (a value of 0 would collapse to black and lose the hue).
Utils::Colors::RGB hueSatToRgb(float h, float s)
{
    return Utils::Colors::hsvToRgb(Utils::Colors::HSV{h, s, 1.f});
}
} // namespace

void Programmer::select(const Utils::Maths::Interval &fids)
{
    m_interval = fids;
}

void Programmer::select(const std::vector<uint16_t> &fids)
{
    m_interval.clear();
    m_interval.add(fids);
}

void Programmer::add(const std::vector<uint16_t> &fids)
{
    m_interval.add(fids);
}

void Programmer::add(const Utils::Maths::Interval &fids)
{
    m_interval |= fids;
}

void Programmer::setColor(const Utils::Colors::RGB &rgb)
{
    m_staticEffects.setColor(m_interval, rgb);
    m_runningEffects.pushRet<Effects::StaticColor>(
        m_interval, m_staticEffects.getColor(m_interval));
}

void Programmer::setIntensity(float v)
{
    m_staticEffects.setIntensity(m_interval, v);
    m_runningEffects.pushRet<Effects::StaticIntensity>(
        m_interval, m_staticEffects.getIntensity(m_interval));
}

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
//     // The programmer is just a layer of effects, replayed every frame -> walk
//     // every stack in order, then every effect within it. An effect only
//     // recomputes when it is due (edited, its selection changed, or its own beat
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
} // namespace LightEngine::Engine::Components
