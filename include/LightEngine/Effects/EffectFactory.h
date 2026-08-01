// #pragma once

// #include <memory>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Effects/Effect.h"
// #include "LightEngine/Effects/EffectSpec.h"

// //
// // EffectFactory: builds a running Effect from a Spec, bound to the given
// group.
// // This is the single construction path - used both when adding an effect
// live
// // in the programmer and when recalling one from a cue.
// //
// namespace LightEngine::Effects::EffectFactory
// {
//     std::unique_ptr<Effect> build(const Spec& spec, DMX::FixtureGroup group);
// }

namespace LightEngine::Effects
{

// Effects on the same selection
struct EffectStack
{
    DMX::FixtureGroup selection;
    std::vector<std::unique_ptr<Effects::Effect>> effects;
    bool dirty = false;
};

struct EffectFactory
{
    EffectStack *c_ptr = nullptr;
    std::vector<EffectStack> stacks;

    void createNew()
    {
        stacks.push_back(EffectStack());
        c_ptr = &stacks.back();
    }

    void clearCurrent()
    {
        auto &v = *c_ptr;
        v = EffectStack();
    }

    EffectFactory() { createNew(); }

    void select(const DMX::FixtureGroup &g)
    {
        if (c_ptr->dirty)
        {
            createNew();
            return;
        }
        c_ptr->selection.add(g);
    }

    void add(const DMX::FixtureGroup &g)
    {

        if (!c_ptr->dirty)
        {
            c_ptr->selection += g;
            return;
        }

        auto groupcp = c_ptr->selection;
        createNew();
        c_ptr->selection += groupcp;
        c_ptr->selection += g;
    }

    template <typename T, typename... Args> void push(Args &&...args)
    {
        c_ptr->effects.push_back(
            std::make_unique<T>(c_ptr->selection, std::forward<Args>(args)...));
    }
};
} // namespace LightEngine::Effects