#pragma once
#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Effect/Static/EffectIntensity.h"
#include "PoolObject.h"
#include <memory>

namespace LightEngine::Engine::Pools
{

class IntensityPreset : public Engine::PoolObject
{
    struct Data
    {
        DMX::FixtureGroup
            m_fg; // the working selection — already does the hard part
        std::shared_ptr<Effects::StaticIntensity> effect;
    };

    Data current;
    std::vector<Data> data;

public:
    IntensityPreset() = default;
    // Color(DMX::FixtureGroup &group, std::shared_ptr<Effects::EffectBase> eff)
    //     : data{std::move(group), std::move(eff)} {};
    // explicit Color(std::vector<std::shared_ptr<Fixtures::>> fixtures)
    // {
    //     m_fg.add(fixtures);
    // }

    // explicit Color(const DMX::FixtureGroup &group) { m_fg.add(group); }

    // direct access — no resolve, it's always live
    // DMX::FixtureGroup &fixtureGroup() { return m_fg; }
    // const DMX::FixtureGroup &fixtureGroup() const { return m_fg; }

    // void setcolor(DMX::FixtureGroup &group, const Utils::Colors::RGB &rgb)
    // {
    //     Data d;
    //     d.m_fg = group;
    //     d.effect = std::make_shared<Effects::StaticIntensity>(rgb);
    //     data.push_back(std::move(d));
    //     // m_fg.add(fx);
    // }

    void setcolor(DMX::FixtureGroup &group,
                  std::shared_ptr<Effects::StaticIntensity> eff)
    {
        Data d;
        d.m_fg = group;
        d.effect = std::move(eff);
        data.push_back(std::move(d));
        // m_fg.add(fx);
    }

    // snapshot of FIDs — only needed for serialization
    // std::vector<uint16_t> fids() const { return m_fg.fids(); }

    std::string describe() const override
    {
        return "Color";
        // return "Group " + std::to_string(number()) + " \"" + name() + "\" " +
        //        m_fg.describe();
    }
};
} // namespace LightEngine::Engine::Pools