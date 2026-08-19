#pragma once
#include "LightEngine/Fixture/Fixture.h"
#include "LightEngine/GDTF/LogicalChannel.h"
#include <initializer_list>
#include <memory>

namespace LightEngine::Engine
{

// struct FixtureAssembly
// {
//     using Fixture = Fixtures::Fixture;
//     using Attribute = GDTF::Attribute;
//     Fixture fixture;
//     uint16_t currentOffset = 0;

// };
// class FixtureBuilder
// {
//     using Fixture = Fixtures::Fixture;
//     using Attribute = GDTF::Attribute;
//     Fixture fixture;
//     uint16_t currentOffset = 0;

//     std::shared_ptr<const GDTF::LogicalChannel> MakeChannel8Bit(Attribute
//     attr,
//                                                                 uint16_t
//                                                                 offset)
//     {
//         GDTF::LogicalChannel lc;
//         lc.attribute = attr;
//         lc.channel = {offset, GDTF::DMXChannel::Resolution::Bit8};
//         lc.functions = {{"", {0, 255}, {0.f, 1.f}}};
//         return std::make_shared<const GDTF::LogicalChannel>(std::move(lc));
//     }

// public:
//     explicit FixtureBuilder(const std::string &name) : fixture(name) {}
//     FixtureBuilder(const std::string &name,
//                    std::initializer_list<Attribute> attributes)
//         : fixture(name)
//     {
//         for (auto &attr : attributes)
//         {
//             fixture.Add(MakeChannel8Bit(attr, currentOffset++));
//         }
//     }

//     void New(const std::string &name)
//     {
//         currentOffset = 0;
//         fixture = Fixture(name);
//     }

//     FixtureBuilder &operator<<(const Attribute &attr)
//     {
//         fixture.Add(MakeChannel8Bit(attr, currentOffset++));
//         return *this;
//     }

//     Fixture Get()
//     {
//         fixture.Build();
//         return fixture;
//     }
// };

class FixtureBuilder
{
    using Attribute = GDTF::Attribute;

    Fixtures::FixtureTemplate m_template;
    uint16_t m_currentOffset = 0;

    std::shared_ptr<GDTF::LogicalChannel> MakeChannel8Bit(Attribute attr,
                                                          uint16_t offset)
    {
        GDTF::LogicalChannel lc;

        lc.attribute = attr;

        lc.channel = {offset, GDTF::DMXChannel::Resolution::Bit8};

        lc.functions = {{"", {0, 255}, {0.f, 1.f}}};

        return std::make_shared<GDTF::LogicalChannel>(std::move(lc));
    }

public:
    explicit FixtureBuilder(const std::string &name) { m_template.name = name; }

    FixtureBuilder &operator<<(Attribute attr)
    {
        Fixtures::ParameterConfig config;

        config.definition = MakeChannel8Bit(attr, m_currentOffset++);

        config.cellIndex = 0;

        m_template.parameters.push_back(std::move(config));

        return *this;
    }

    std::shared_ptr<Fixtures::FixtureTemplate> Get() const
    {
        return std::make_shared<Fixtures::FixtureTemplate>(m_template);
    }
};
} // namespace LightEngine::Engine