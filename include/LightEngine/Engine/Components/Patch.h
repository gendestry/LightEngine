#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "LightEngine/DMX/UniversePatch.h"
#include "LightEngine/Fixture/Fixture.h"
#include "Utils/Logging/Logger.h"
#include "Utils/Traits/Describe.h"

namespace LightEngine::Engine::Components
{
class Patch : public Utils::Traits::Describe
{
    struct FixtureStatus : Utils::Traits::Stringify
    {
        uint16_t universe = 0;
        uint32_t patchInfoID = 0;

        FixtureStatus() = default;
        FixtureStatus(uint16_t universe, uint32_t patchInfoID)
            : universe(universe), patchInfoID(patchInfoID) {}

        [[nodiscard]] std::string toString() const override
        {
            return std::format("uni:{} frag:{}", universe, patchInfoID);
        }
    };

    struct FixPatch
    {
        uint16_t universe = 0;
        DMX::PatchInfo info;
    };

    using FixturePtr = std::shared_ptr<LightEngine::Fixtures::Fixture>;

    Utils::Logger logger;

    uint64_t m_fixCurrentID = 0;
    std::map<uint64_t, FixturePtr> m_fixtureByUID;
    std::map<uint64_t, FixtureStatus> m_fixtureStatus;
    std::unordered_map<uint16_t, std::vector<uint64_t>> m_fixtureByFIDs;

    std::map<uint16_t, LightEngine::DMX::UniversePatch> m_universes; // by universe id

    DMX::UniversePatch &getUniverse(uint16_t universe);

    std::optional<FixtureStatus> isPatched(uint32_t id) const;
    std::optional<FixPatch> patchInfo(uint32_t id) const;

public:
    Patch() : logger("Patch") { logger.setLoggerLevel(Utils::Logger::DEBUGGING); };

    std::vector<FixturePtr> addFixtures(const Fixtures::Fixture &fixture, uint32_t amount = 1);
    bool removeFixture(uint32_t id);

    bool patch(uint32_t fixUid, uint16_t universe, uint32_t addr);
    bool patch(const std::vector<uint32_t> &fixUids, uint16_t universe, uint32_t addr);
    std::vector<uint16_t> patch(Fixtures::Fixture *fixture, uint16_t universe,
                                uint16_t amount, std::optional<uint32_t> start = std::nullopt,
                                std::optional<uint16_t> startFID = std::nullopt);

    bool unpatch(uint32_t id);
    uint32_t unpatch(const std::vector<uint32_t> &ids);

    [[nodiscard]] std::vector<uint64_t> sortedByAddress() const;

    void clearDMXBuffers();

    [[nodiscard]] FixturePtr getFixture(uint64_t uid) const;
    [[nodiscard]] std::vector<FixturePtr> fixturesByFID(uint16_t fid) const;
    [[nodiscard]] const std::map<uint64_t, FixturePtr> &fixturesByUID() const
    {
        return m_fixtureByUID;
    }

    const std::map<uint16_t, LightEngine::DMX::UniversePatch> &universes() const
    {
        return m_universes;
    }

    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] std::string uniDumpStr() const;
    virtual void print() const override;
};
} // namespace LightEngine::Engine::Components