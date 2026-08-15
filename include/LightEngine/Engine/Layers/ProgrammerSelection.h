#pragma once

// #include <memory>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Effect/Animated/EffectDimmerChase.h"
#include "LightEngine/Effect/EffectBase.h"
#include "LightEngine/Effect/Static/EffectColor.h"
#include "LightEngine/Effect/Static/EffectIntensity.h"

namespace LightEngine::Engine
{

// Effects on the same selectio

struct ProgrammerSelection
{

    DMX::FixtureGroup selection;
    bool dirty = false;

    // EffectStack *c_ptr = nullptr;
    // std::vector<EffectStack> stacks;
    // std::vector<EffectStack> stacks;

    void reset()
    {
        selection = DMX::FixtureGroup();
        dirty = false;
        // stacks.push_back(EffectStack());
        // c_ptr = &stacks.back();
    }

    // void clearCurrent()
    // {
    //     auto &v = *c_ptr;
    //     v = EffectStack();
    // }

    ProgrammerSelection() { reset(); }

    void select(const DMX::FixtureGroup &g)
    {
        if (dirty)
        {
            reset();
            return;
        }

        selection.add(g);
    }

    void add(const DMX::FixtureGroup &g)
    {

        if (!dirty)
        {
            selection += g;
            return;
        }

        // auto groupcp = c_ptr->selection;
        // createNew();
        // c_ptr->selection += groupcp;
        // c_ptr->selection += g;
    }

    // template <typename T, typename... Args> void push(Args &&...args)
    // {
    //     c_ptr->effects.push_back(
    //         std::make_unique<T>(std::forward<Args>(args)...));
    // }
};
} // namespace LightEngine::Engine