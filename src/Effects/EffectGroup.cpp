#include "LightEngine/Effects/EffectGroup.h"

namespace LightEngine::Effects
{
Utils::Maths::Interval EffectGroup::animatedCoverage(EffectCategory cat) const
{
    Utils::Maths::Interval mask;
    for (const auto &wrapper : applied)
    {
        const auto &effect = wrapper->effect;
        if (effect->enabled() && effect->GetEffectType() == EffectType::ANIMATED &&
            effect->GetEffectCategory() == cat)
        {
            mask |= wrapper->group;
        }
    }
    return mask;
}

std::vector<std::pair<uint16_t, Engine::FixtureValues>>
EffectGroup::snapshotStatic(const Utils::Maths::Interval &sel, EffectCategory cat,
                            const Utils::Time::TimeContext &t)
{
    // Fixtures a running animated effect owns are excluded outright, rather
    // than storing whatever static value sits underneath it - predictable
    // regardless of that effect's merge policy (an HTP chase lifts a level
    // without replacing it, so there IS a static value under it; we still
    // don't want it in the preset).
    Utils::Maths::Interval targets = sel;
    targets -= animatedCoverage(cat);
    if (targets.empty())
    {
        return {};
    }

    // Replay every enabled static effect into a scratch frame, in the same
    // order apply() would - this reproduces exactly what the programmer is
    // showing right now, not a re-derivation of it. Each effect gets its own
    // (unmasked) group so its live cache is used/refreshed normally; the
    // masking happens afterwards, when reading the frame back.
    Engine::Frame frame;
    for (auto &wrapper : applied)
    {
        auto &effect = wrapper->effect;
        if (!effect->enabled() || effect->GetEffectType() != EffectType::STATIC)
        {
            continue;
        }
        if (effect->due(t.now, wrapper->group))
        {
            effect->evaluate(t, wrapper->group);
        }
        effect->replay(frame);
    }

    std::vector<std::pair<uint16_t, Engine::FixtureValues>> out;
    out.reserve(targets.size());
    for (const uint64_t fid64 : targets.values())
    {
        const auto fid = static_cast<uint16_t>(fid64);
        const auto *values = frame.get(fid);
        if (!values)
        {
            continue;
        }

        // Project only the attribute this preset type cares about - a
        // fixture with no value for it is simply not stored.
        Engine::FixtureValues picked;
        switch (cat)
        {
        case EffectCategory::COLOR:
            if (!values->color)
            {
                continue;
            }
            picked.color = values->color;
            break;
        case EffectCategory::DIMMER:
            if (!values->intensity)
            {
                continue;
            }
            picked.intensity = values->intensity;
            break;
        }
        out.emplace_back(fid, std::move(picked));
    }
    return out;
}
} // namespace LightEngine::Effects
