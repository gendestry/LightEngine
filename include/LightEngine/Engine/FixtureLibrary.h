#pragma once
#include "FixtureBuilder.h"
#include "LightEngine/Fixture/Fixture.h"
#include "LightEngine/Fixture/FixtureTemplate.h"

namespace LightEngine::Engine
{
struct Vendor
{
    std::map<std::string, std::shared_ptr<Fixtures::Fixture>> fixtures;
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

        vendor.fixtures["RGB"] = std::make_shared<Fixtures::Fixture>(fixTemp);
    }

public:
    FixtureLibrary() { addDefaults(); }

    std::shared_ptr<Fixtures::Fixture> find(const std::string &name)
    {
        auto it = vendor.fixtures.find(name);

        if (it != vendor.fixtures.end())
        {
            return it->second; // the Fixture
        }

        return nullptr;
    };
};
} // namespace LightEngine::Engine