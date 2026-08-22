#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "LightEngine/DMX/Universe.h"
#include "LightEngine/Fixture/Fixture.h"
#include "LightEngine/Fixture/FixtureTemplate.h"
#include "Utils/Logging/Logger.h"

namespace LightEngine::Engine::Components
{
class Patch : public Utils::Traits::Stringify
{
    using FixturePtr = std::shared_ptr<LightEngine::Fixtures::Fixture>;

    Utils::Logger logger;

    std::map<uint64_t, FixturePtr> m_fixtureByUID;
    std::unordered_map<uint16_t, std::vector<uint64_t>> m_fixtureByFIDs;

    std::map<uint16_t, LightEngine::DMX::Universe> m_universes; // by universe id

    DMX::Universe &getUniverse(uint16_t universe);
    std::vector<FixturePtr> addFixtures(const Fixtures::Fixture &fixture, uint32_t amount = 1);

public:
    Patch() : logger("Patch") { logger.setLoggerLevel(Utils::Logger::DEBUGGING); };
    std::vector<uint16_t> patch(const Fixtures::Fixture &fixture, uint16_t universe,
                                uint16_t amount, std::optional<uint32_t> start = std::nullopt,
                                std::optional<uint16_t> startFID = std::nullopt);

    // temp
    const std::map<uint64_t, FixturePtr> &fixs()
    {
        return m_fixtureByUID;
    }

    [[nodiscard]] std::string toString() const override;
};
} // namespace LightEngine::Engine::Components