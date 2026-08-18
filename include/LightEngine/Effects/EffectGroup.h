#pragma once
#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Effects/EffectBase.h"
#include <memory>
#include <vector>

namespace LightEngine::Effects
{
struct EffectWrapper
{
    DMX::FixtureGroup group;
    std::shared_ptr<Effects::EffectBase> effect;
};
class EffectGroup
{
    std::vector<std::shared_ptr<EffectWrapper>> applied;

public:
    void push(std::shared_ptr<EffectWrapper> eff)
    {
        applied.push_back(std::move(eff));
    }

    template <typename T, typename... Args>
    void push(DMX::FixtureGroup group, Args &&...args)
    {
        EffectWrapper wrap;
        wrap.group = std::move(group);
        wrap.effect = std::make_shared<T>(std::forward<Args>(args)...);
        applied.emplace_back(std::make_shared<EffectWrapper>(wrap));
    }

    template <typename T, typename... Args>
    std::shared_ptr<EffectWrapper> pushRet(DMX::FixtureGroup group,
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
};
} // namespace LightEngine::Effects