#pragma once
#include "FixtureBuilder.h"
#include "LightEngine/Fixture/Fixture.h"
#include "LightEngine/Fixture/FixtureTemplate.h"

namespace LightEngine::Engine
{
struct Vendor
{
    std::map<std::string, Fixtures::Fixture> fixtures;
    std::map<std::string, std::shared_ptr<Fixtures::FixtureTemplate>> fixtureTemplates;
};
class FixtureLibrary
{
    Vendor vendor;

    void addDefaults()
    {
        std::shared_ptr<Fixtures::FixtureTemplate> fixTemp = std::make_shared<Fixtures::FixtureTemplate>("RGB");
        std::vector<Fixtures::ParameterConfig> parameters;
        parameters.emplace_back(FixtureBuilder::MakeChannel8Bit(GDTF::Attribute::COLOR_R, 0), 0);
        parameters.emplace_back(FixtureBuilder::MakeChannel8Bit(GDTF::Attribute::COLOR_G, 1), 0);
        parameters.emplace_back(FixtureBuilder::MakeChannel8Bit(GDTF::Attribute::COLOR_B, 2), 0);
        fixTemp->parameters = std::move(parameters);
        vendor.fixtureTemplates["RGB"] = fixTemp;

        vendor.fixtures["RGB"] = std::move(Fixtures::Fixture(fixTemp));

        // add({"RGB",
        //      {GDTF::Attribute::COLOR_R, GDTF::Attribute::COLOR_G,
        //       GDTF::Attribute::COLOR_B}});
    }

public:
    FixtureLibrary() { addDefaults(); }
    // void add(FixtureBuilder &builder)
    // {
    //     auto fix = builder.Get();
    //     vendor.fixtures[fix.Name()] = std::move(fix);
    // }

    // void add(FixtureBuilder &&builder)
    // {
    //     auto fix = builder.Get();
    //     vendor.fixtures[fix.Name()] = std::move(fix);
    // }
    // void add(FixtureBuilder builder)
    // {
    //     auto fix = builder.Get();
    //     vendor.fixtures[fix.Name()] = std::move(fix);
    // }

    // std::optional<Fixtures::Fixture> find(const std::string &name)
    // {
    //     auto it = vendor.fixtures.find(name);

    //     if (it != vendor.fixtures.end())
    //     {
    //         return it->second; // the Fixture
    //     }

    //     return std::nullopt;
    // };

    std::optional<Fixtures::Fixture> find(const std::string &name)
    {
        auto it = vendor.fixtures.find(name);

        if (it != vendor.fixtures.end())
        {
            return it->second; // the Fixture
        }

        return std::nullopt;
    };

    std::optional<std::shared_ptr<Fixtures::FixtureTemplate>> findTemplate(const std::string &name)
    {
        auto it = vendor.fixtureTemplates.find(name);

        if (it != vendor.fixtureTemplates.end())
        {
            return it->second; // the Fixture
        }

        return std::nullopt;
    };
    // std::
};
} // namespace LightEngine::Engine