#pragma once
#include "LightEngine/Effects/EffectBase.h"
#include "LightEngine/Engine/Layers/Layer.h"
#include <memory>
#include <vector>

namespace LightEngine::Effects
{
struct EffectWrapper
{
    Utils::Maths::Interval group;
    std::shared_ptr<Effects::EffectBase> effect;
};
class EffectGroup : public LightEngine::Engine::Layer
{
    // using LightEngine::Engine::Layer;
    std::vector<std::shared_ptr<EffectWrapper>> applied;

public:
    EffectGroup(Priority prio = Priority::NORMAL) : LightEngine::Engine::Layer(prio) {}
    void push(std::shared_ptr<EffectWrapper> eff)
    {
        applied.push_back(std::move(eff));
    }

    template <typename T, typename... Args>
    void push(Utils::Maths::Interval group, Args &&...args)
    {
        EffectWrapper wrap;
        wrap.group = std::move(group);
        wrap.effect = std::make_shared<T>(std::forward<Args>(args)...);
        applied.emplace_back(std::make_shared<EffectWrapper>(wrap));
    }

    template <typename T, typename... Args>
    std::shared_ptr<EffectWrapper> pushRet(Utils::Maths::Interval group,
                                           Args &&...args)
    {
        EffectWrapper wrap;
        wrap.group = std::move(group);
        wrap.effect = std::make_shared<T>(std::forward<Args>(args)...);

        auto ret = std::make_shared<EffectWrapper>(wrap);
        applied.push_back(ret);
        return ret;
    }

    void clear() { applied.clear(); }

    std::vector<std::shared_ptr<EffectWrapper>> &getEffects()
    {
        return applied;
    }

    std::optional<std::shared_ptr<EffectWrapper>> lastEffect()
    {
        if (applied.empty())
        {
            return std::nullopt;
        }
        return applied.back();
    }

    std::optional<std::shared_ptr<EffectWrapper>> operator[](std::size_t index)
    {
        if (index < applied.size())
        {
            return applied[index];
        }

        return std::nullopt;
    }

    void apply(LightEngine::Engine::Frame &frame, const Utils::Time::TimeContext &time) override
    {
        for (auto &wrapper : applied)
        {
            auto &group = wrapper->group;
            auto &effect = wrapper->effect;
            if (!effect->enabled())
            {
                continue;
            }
            if (effect->due(time.now, group))
            {
                effect->evaluate(time, group);
            }
            effect->replay(frame);
        }
    }

    // The union of every enabled ANIMATED effect's group in this category.
    // Used by snapshotStatic() to exclude fixtures a running effect owns from
    // a store - the static value underneath a chase isn't what's on stage.
    [[nodiscard]] Utils::Maths::Interval animatedCoverage(EffectCategory cat) const;

    // The static (non-animated) picture of `sel`, restricted to fixtures no
    // animated effect in `cat` currently covers. Replays every enabled STATIC
    // effect of this group into a scratch frame (LTP-by-order, same as apply())
    // and reads back only the entries `sel` asks for. This is what a preset
    // stores: the store algorithm, not a rendering path.
    [[nodiscard]] std::vector<std::pair<uint16_t, LightEngine::Engine::FixtureValues>>
    snapshotStatic(const Utils::Maths::Interval &sel, EffectCategory cat,
                   const Utils::Time::TimeContext &t);
};
} // namespace LightEngine::Effects