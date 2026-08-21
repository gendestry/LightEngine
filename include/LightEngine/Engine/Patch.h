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
// #include "LightEngine/Engine/FixtureLibrary.h"   // TODO: library + loadGDTF

//
// Patch: owns the universes and the mapping from Fixture ID (FID) to the live
// fixture instances placed in them. Patching a fixture means copying a
// definition into a universe (which wires it to the DMX buffer) and assigning
// it one or more FIDs. Patch is the single source of truth for which universes
// have changed (dirty set) so the output stage knows what to send.
//
namespace LightEngine::Engine
{
// class PatchSelection : public Utils::Traits::Stringify
// {
//     using FixturePtr = std::shared_ptr<LightEngine::Fixtures::Fixture>;
//     std::map<uint64_t, FixturePtr> fixtures;

// public:
//     void selectTemplate(FixturePtr ptr)
//     {
//         selectedTemplate = ptr;
//     }
// };

// struct PatchData
// {
//     using FixturePtr = std::shared_ptr<LightEngine::Fixtures::Fixture>;
//     std::map<uint64_t, FixturePtr> fixtures;
// };
class Patch : public Utils::Traits::Stringify
{
    using FixturePtr = std::shared_ptr<LightEngine::Fixtures::Fixture>;

    // FixtureLibrary                            m_library;   // TODO
    std::map<uint64_t, FixturePtr> m_fixtureByUID;
    std::map<uint16_t, LightEngine::DMX::Universe>
        m_universes;                           // by universe id
    std::map<uint16_t, FixturePtr> m_fixtures; // by FID
    std::unordered_map<std::string, std::vector<FixturePtr>> m_byName;
    std::set<uint16_t> m_usedFids;
    std::set<uint16_t> m_dirty; // universes needing output

    Utils::Logger logger;

    LightEngine::DMX::Universe &ensureUniverse(uint16_t universe);
    [[nodiscard]] uint16_t nextFreeFid() const;
    void registerFixture(uint16_t fid, const FixturePtr &fixture);

public:
    Patch() : logger("Patch") {}

    std::vector<uint16_t>
    patch(const LightEngine::Fixtures::Fixture &fixture, uint16_t universe,
          uint16_t amount, std::optional<uint32_t> start = std::nullopt,
          std::optional<uint16_t> startFID = std::nullopt);

    // ---- lookup ----
    [[nodiscard]] LightEngine::DMX::Universe *getUniverse(uint16_t universe);
    [[nodiscard]] const std::map<uint16_t, LightEngine::DMX::Universe> &
    universes() const
    {
        return m_universes;
    }

    [[nodiscard]] FixturePtr getFixtureByID(uint64_t id) const;

    [[nodiscard]] FixturePtr getFixture(uint16_t fid) const;
    [[nodiscard]] std::vector<FixturePtr>
    getFixtures(const std::vector<uint16_t> &fids) const;
    [[nodiscard]] const std::vector<FixturePtr> &
    getFixturesByName(const std::string &name) const;
    [[nodiscard]] const std::map<uint16_t, FixturePtr> &fixtures() const
    {
        return m_fixtures;
    }

    // Zero every universe's DMX values (keeps the patch). Frame-start reset.
    void blackout()
    {
        for (auto &[id, uni] : m_universes)
        {
            uni.blackout();
        }
    }

    // ---- dirty tracking ----
    [[nodiscard]] const std::set<uint16_t> &dirtyUniverses() const
    {
        return m_dirty;
    }
    void markDirty(uint16_t universe) { m_dirty.insert(universe); }
    void clearDirty() { m_dirty.clear(); }

    [[nodiscard]] std::string toString() const override;
};
} // namespace LightEngine::Engine
