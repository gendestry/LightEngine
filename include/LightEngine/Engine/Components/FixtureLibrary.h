#pragma once
#include "LightEngine/Fixture/Fixture.h"
#include "LightEngine/Fixture/FixtureTemplate.h"
#include "LightEngine/FixtureTools/ParameterBuilder.h"
#include <unordered_map>

namespace LightEngine::Engine::Components
{

class FixtureLibrary
{
    struct Vendor
    {
        std::map<std::string, std::shared_ptr<Fixtures::Fixture>> fixtures;
        std::unordered_map<std::string, Fixtures::FixtureTemplate> templates;
    };

    Vendor vendor;

    void addDefaults()
    {
        using GDTF::Attribute;
        std::shared_ptr<Fixtures::FixtureTemplate> fixTemp = std::make_shared<Fixtures::FixtureTemplate>("RGB");
        FixtureTools::ParameterCell parCell(0, 0);
        parCell << GDTF::Attribute::COLOR_R << GDTF::Attribute::COLOR_G << GDTF::Attribute::COLOR_B;
        fixTemp->parameters = std::move(parCell.get());

        // vendor.templates["RGB"] =
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
} // namespace LightEngine::Engine::Components