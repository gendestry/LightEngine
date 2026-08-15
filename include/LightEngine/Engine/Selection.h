#pragma once
#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Effect/EffectBase.h"
#include <memory>

namespace LightEngine::Engine
{
struct EffectWrapper
{
    DMX::FixtureGroup group;
    std::shared_ptr<Effects::EffectBase> effect;
};
class EffectGroup
{
    std::vector<EffectWrapper> applied;

public:
    template <typename T, typename... Args>
    void push(DMX::FixtureGroup group, Args &&...args)
    {
        EffectWrapper wrap;
        wrap.group = std::move(group);
        wrap.effect = std::make_shared<T>(std::forward<Args>(args)...);
        applied.push_back(std::move(wrap));
        // c_ptr->effects.push_back(
        // std::make_unique<T>(std::forward<Args>(args)...));
    }
};

class Selection
{
};

struct EffectHolder
{
    struct EffectStack
    {
        DMX::FixtureGroup selection;
        std::vector<std::unique_ptr<Effects::EffectBase>> effects;
        bool dirty = false;
    };

    EffectStack *c_ptr = nullptr;
    std::vector<EffectStack> stacks;
    // std::vector<EffectStack> stacks;

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

    EffectHolder() { createNew(); }

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
            std::make_unique<T>(std::forward<Args>(args)...));
    }
};

// // Effects on the same selection

// // struct StaticEffectStack
// // {
// //     DMX::FixtureGroup selection;
// //     // DMX::FixtureGroup selection;
// //     std::vector<std::unique_ptr<Effects::Effect>> effects;

// //     bool dirty = false;
// // };

// class Selection
// {
//     DMX::FixtureGroup selection;
//     Patch &m_patch;

//     void createNew()
//     {
//         selection = DMX::FixtureGroup();
//         // stacks.push_back(EffectStack());
//         // c_ptr = &stacks.back();
//     }

// public:
//     bool dirty = false;
//     Selection(Patch &patch) : m_patch(patch) { createNew(); }

//     void select(const DMX::FixtureGroup &g)
//     {
//         selection += g;
//         // selection.add(g);
//         // if (c_ptr->dirty)
//         // {
//         //     createNew();
//         //     return;
//         // }
//         // c_ptr->selection.add(g);
//     }

//     void select(const std::vector<uint16_t> &fids)
//     {
//         // selection += m_patch.getFixtures(fids);
//         selection.add(m_patch.getFixtures(fids));
//         // selection.add(g);
//         // if (c_ptr->dirty)
//         // {
//         //     createNew();
//         //     return;
//         // }
//         // c_ptr->selection.add(g);
//     }

//     // void add(const DMX::FixtureGroup &g)
//     // {
//     //     selection.add(g);

//     //     // if (!c_ptr->dirty)
//     //     // {
//     //     //     c_ptr->selection += g;
//     //     //     return;
//     //     // }

//     //     // auto groupcp = c_ptr->selection;
//     //     // createNew();
//     //     // c_ptr->selection += groupcp;
//     //     // c_ptr->selection += g;
//     // }

//     void clear() { createNew(); }

//     auto get() { return selection.fixtures(); }
// };
} // namespace LightEngine::Engine