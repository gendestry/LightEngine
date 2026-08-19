#pragma once
#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Effects/EffectGroup.h"
#include "LightEngine/Effects/Static/EffectIntensity.h"
#include "Utils/Colors/HSV.h"
#include <unordered_map>

namespace LightEngine::Engine
{

struct All
{
    std::map<uint16_t, FixtureValues> values;
    // std::vector<Effects::EffectGroup> groups;
    // std::unordered_map<Effects::EffectCategory, typename Tp>
};

class GroupSelection
{
    DMX::FixtureGroup m_selected;

public:
    void select(const DMX::FixtureGroup &g) { m_selected = g; }

    // void select(const std::vector<uint16_t> &fids)
    // {
    //     DMX::FixtureGroup g(m_patch.getFixtures(fids));
    //     m_selected = std::move(g);
    // }

    void add(const DMX::FixtureGroup &g) { m_selected += g; }

    [[nodiscard]] const DMX::FixtureGroup &get() const { return m_selected; }

    // void clearSelection()

    // void add(const std::vector<uint16_t> &fids)
    // {
    //     m_selected += m_patch.getFixtures(fids);
    // }
    //     };
};

class StaticEffectHolder
{
    // GroupSelection& selection;
    // std::vector<std::shared_ptr<Effects::EffectWrapper>> groups;
    // std::list<std::shared_ptr<Effects::EffectWrapper>> staticColors;
    // std::list<std::shared_ptr<Effects::EffectWrapper>> staticIntensity;
    // std::unordered_map<Effects::EffectCategory, std::set<uint16_t>> byType;
    std::map<uint16_t, FixtureValues> values;
    std::unordered_map<Effects::EffectCategory, std::set<uint16_t>> byType;

    GroupSelection &selection;

    std::vector<std::pair<uint16_t, FixtureValues>>
    get(Effects::EffectCategory cat, const DMX::FixtureGroup &group)
    {
        std::vector<std::pair<uint16_t, FixtureValues>> result;
        result.reserve(byType[cat].size());

        FixtureValues v;
        Utils::Colors::HSV hsvCol;

        for (const auto id : group.fids())
        {
            if (const auto it = values.find(id); it != values.end())
            {
                switch (cat)
                {
                case Effects::EffectCategory::COLOR:
                    hsvCol.h = it->second.color->h;
                    hsvCol.s = it->second.color->s;
                    v.color = hsvCol;
                    break;
                case Effects::EffectCategory::DIMMER:
                    v.intensity = it->second.intensity;
                    break;
                }
                result.push_back({id, std::move(v)});
            }
        }

        return result;
    }

public:
    StaticEffectHolder(GroupSelection &gselection) : selection(gselection) {}
    void setIntensity(const DMX::FixtureGroup &group, float i)
    {
        for (auto g : group.fids())
        {
            values[g].intensity = i;
            byType[Effects::EffectCategory::DIMMER].emplace(g);
        }
    }

    void setColor(const DMX::FixtureGroup &group,
                  const Utils::Colors::RGB &color)
    {
        auto hsv = color.toHSV();

        for (auto &g : group.fids())
        {
            values[g].color = hsv;
            byType[Effects::EffectCategory::COLOR].emplace(g);
        }
    }

    std::vector<std::pair<uint16_t, FixtureValues>>
    getIntensity(const DMX::FixtureGroup &group)
    {
        return get(Effects::EffectCategory::DIMMER, group);
    }

    std::vector<std::pair<uint16_t, FixtureValues>>
    getColor(const DMX::FixtureGroup &group)
    {
        return get(Effects::EffectCategory::COLOR, group);
    }

    const std::map<uint16_t, FixtureValues> &getValues() const
    {
        return values;
    }

    void clear()
    {
        values.clear();
        byType.clear();
    }
};
} // namespace LightEngine::Engine

// #pragma once
// #include "LightEngine/DMX/FixtureGroup.h"
// #include "LightEngine/Effect/EffectBase.h"
// #include <memory>

// namespace LightEngine::Engine
// {
// struct EffectWrapper
// {
//     const DMX::FixtureGroup& group;
//     std::shared_ptr<Effects::EffectBase> effect;
// };
// class EffectGroup
// {
//     std::vector<EffectWrapper> applied;

// public:
//     template <typename T, typename... Args>
//     void push(const DMX::FixtureGroup& group, Args &&...args)
//     {
//         EffectWrapper wrap;
//         wrap.group = std::move(group);
//         wrap.effect = std::make_shared<T>(std::forward<Args>(args)...);
//         applied.push_back(std::move(wrap));
//         // c_ptr->effects.push_back(
//         // std::make_unique<T>(std::forward<Args>(args)...));
//     }
// };

// class Selection
// {
// };

// struct EffectHolder
// {
//     struct EffectStack
//     {
//         DMX::FixtureGroup selection;
//         std::vector<std::unique_ptr<Effects::EffectBase>> effects;
//         bool dirty = false;
//     };

//     EffectStack *c_ptr = nullptr;
//     std::vector<EffectStack> stacks;
//     // std::vector<EffectStack> stacks;

//     void createNew()
//     {
//         stacks.push_back(EffectStack());
//         c_ptr = &stacks.back();
//     }

//     void clearCurrent()
//     {
//         auto &v = *c_ptr;
//         v = EffectStack();
//     }

//     EffectHolder() { createNew(); }

//     void select(const DMX::FixtureGroup &g)
//     {
//         if (c_ptr->dirty)
//         {
//             createNew();
//             return;
//         }
//         c_ptr->selection.add(g);
//     }

//     void add(const DMX::FixtureGroup &g)
//     {

//         if (!c_ptr->dirty)
//         {
//             c_ptr->selection += g;
//             return;
//         }

//         auto groupcp = c_ptr->selection;
//         createNew();
//         c_ptr->selection += groupcp;
//         c_ptr->selection += g;
//     }

//     template <typename T, typename... Args> void push(Args &&...args)
//     {
//         c_ptr->effects.push_back(
//             std::make_unique<T>(std::forward<Args>(args)...));
//     }
// };

// // // Effects on the same selection

// // // struct StaticEffectStack
// // // {
// // //     DMX::FixtureGroup selection;
// // //     // DMX::FixtureGroup selection;
// // //     std::vector<std::unique_ptr<Effects::Effect>> effects;

// // //     bool dirty = false;
// // // };

// // class Selection
// // {
// //     DMX::FixtureGroup selection;
// //     Patch &m_patch;

// //     void createNew()
// //     {
// //         selection = DMX::FixtureGroup();
// //         // stacks.push_back(EffectStack());
// //         // c_ptr = &stacks.back();
// //     }

// // public:
// //     bool dirty = false;
// //     Selection(Patch &patch) : m_patch(patch) { createNew(); }

// //     void select(const DMX::FixtureGroup &g)
// //     {
// //         selection += g;
// //         // selection.add(g);
// //         // if (c_ptr->dirty)
// //         // {
// //         //     createNew();
// //         //     return;
// //         // }
// //         // c_ptr->selection.add(g);
// //     }

// //     void select(const std::vector<uint16_t> &fids)
// //     {
// //         // selection += m_patch.getFixtures(fids);
// //         selection.add(m_patch.getFixtures(fids));
// //         // selection.add(g);
// //         // if (c_ptr->dirty)
// //         // {
// //         //     createNew();
// //         //     return;
// //         // }
// //         // c_ptr->selection.add(g);
// //     }

// //     // void add(const DMX::FixtureGroup &g)
// //     // {
// //     //     selection.add(g);

// //     //     // if (!c_ptr->dirty)
// //     //     // {
// //     //     //     c_ptr->selection += g;
// //     //     //     return;
// //     //     // }

// //     //     // auto groupcp = c_ptr->selection;
// //     //     // createNew();
// //     //     // c_ptr->selection += groupcp;
// //     //     // c_ptr->selection += g;
// //     // }

// //     void clear() { createNew(); }

// //     auto get() { return selection.fixtures(); }
// // };
// } // namespace LightEngine::Engine