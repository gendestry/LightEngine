#include "LightEngine/Engine/Components/Programmer.h"

#include "LightEngine/Effects/Animated/EffectDimmerChase.h"
#include "LightEngine/Effects/Static/EffectColor.h"
#include "LightEngine/Effects/Static/EffectIntensity.h"

namespace LightEngine::Engine::Components
{

void FixtureSelection::select(const Utils::Maths::Interval &fids)
{
    if (fids.empty())
    {
        logger.warn("Selecting 0 fixtures");
    }
    m_interval = fids;
    logger.info("Selected fixtures: {}", m_interval.toString());
}

void FixtureSelection::select(const std::vector<uint16_t> &fids)
{
    m_interval.clear();
    if (fids.empty())
    {
        logger.warn("Selecting 0 fixtures");
    }
    m_interval.add(fids);
    logger.info("Selected fixtures: {}", m_interval.toString());
}

void FixtureSelection::add(const std::vector<uint16_t> &fids)
{
    if (fids.empty())
    {
        logger.warn("Adding 0 to selection fixtures");
    }
    m_interval.add(fids);
    logger.info("Selected fixtures: {}", m_interval.toString());
}

void FixtureSelection::add(const Utils::Maths::Interval &fids)
{
    if (fids.empty())
    {
        logger.warn("Adding 0 to selection fixtures");
    }
    m_interval |= fids;
    logger.info("Selected fixtures: {}", m_interval.toString());
}


void Programmer::applyEffect(std::shared_ptr<Effects::EffectWrapper> eff)
{
    m_runningEffects.push(std::move(eff));
}

void Programmer::setColor(const Utils::Colors::RGB &rgb)
{
    logger.info("Setting color: {}", rgb.toString());
    m_runningEffects.pushRet<Effects::StaticColor>(m_selection.get(), rgb);
}

void Programmer::setIntensity(float v)
{
    logger.info("Setting intensity: {}", v);
    m_runningEffects.pushRet<Effects::StaticIntensity>(m_selection.get(), v);
}

void Programmer::addDimmerChase()
{
    m_runningEffects.pushRet<Effects::DimmerChase>(m_selection.get(), Utils::Maths::SINUSOID, 60.f, 1.f, m_selection.get().size());
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

void Programmer::apply(Frame &frame, const Utils::Time::TimeContext &time)
{
    // The programmer is just a layer of effects, replayed every frame -> walk
    // every stack in order, then every effect within it. An effect only
    // recomputes when it is due (edited, its selection changed, or its own beat
    // grid says a new step arrived); otherwise its cached output is
    // replayed.
    m_runningEffects.apply(frame, time);
}
} // namespace LightEngine::Engine::Components
