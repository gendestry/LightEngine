#pragma once
#include "LightEngine/Fixture/Fixture.h"
#include "LightEngine/Fixture/FixtureTemplate.h"
#include "LightEngine/FixtureTools/ParameterBuilder.h"

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
        using GDTF::Attribute;
        std::shared_ptr<Fixtures::FixtureTemplate> fixTemp = std::make_shared<Fixtures::FixtureTemplate>("RGB");
        FixtureTools::ParameterCell parCell(0, 0);
        parCell << GDTF::Attribute::COLOR_R << GDTF::Attribute::COLOR_G << GDTF::Attribute::COLOR_B;
        fixTemp->parameters = std::move(parCell.get());

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