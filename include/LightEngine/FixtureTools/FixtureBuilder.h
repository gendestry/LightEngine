// #pragma once
// #include "LightEngine/Fixture/Fixture.h"
// #include "LightEngine/GDTF/LogicalChannel.h"
// #include <initializer_list>
// #include <memory>

// namespace LightEngine::FixtureTools
// {

// class FixtureBuilder
// {
//     using Fixture = LightEngine::Fixtures::Fixture;
//     using Attribute = GDTF::Attribute;
//     Fixture fixture;
//     uint16_t currentOffset = 0;

// public:
//     static std::shared_ptr<const GDTF::LogicalChannel> MakeChannel8Bit(Attribute attr,
//                                                                        uint16_t offset)
//     {
//         GDTF::LogicalChannel lc;
//         lc.attribute = attr;
//         lc.channel = {offset, GDTF::DMXChannel::Resolution::Bit8};
//         lc.functions = {{"", {0, 255}, {0.f, 1.f}}};
//         return std::make_shared<const GDTF::LogicalChannel>(std::move(lc));
//     }

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
// } // namespace LightEngine::FixtureTools

// /*static Fixtures::FixtureTemplate makeTemplate(const std::string &name, std::initializer_list<Attribute> attributes)
//     {
//         Fixtures::FixtureTemplate temp(name);
//         std::vector<Fixtures::ParameterConfig> parameters;
//         unsigned int offset = 0;
//         for(auto& att : attributes)
//         {
//             parameters.emplace_back(MakeChannel8Bit(att, offset++), 0);
//         }
//     }*/