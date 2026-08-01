#pragma once
#include "FixtureBuilder.h"
#include "LightEngine/Fixture/Fixture.h"

namespace LightEngine::Engine
{
struct Vendor
{
    std::map<std::string, Fixtures::Fixture> fixtures;
};
class FixtureLibrary
{
    Vendor vendor;

    void addDefaults()
    {
        add({"RGB",
             {GDTF::Attribute::COLOR_R, GDTF::Attribute::COLOR_G,
              GDTF::Attribute::COLOR_B}});
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
    void add(FixtureBuilder builder)
    {
        auto fix = builder.Get();
        vendor.fixtures[fix.Name()] = std::move(fix);
    }

    std::optional<Fixtures::Fixture> find(const std::string &name)
    {
        auto it = vendor.fixtures.find(name);

        if (it != vendor.fixtures.end())
        {
            return it->second; // the Fixture
        }

        return std::nullopt;
    };
    // std::
};
} // namespace LightEngine::Engine