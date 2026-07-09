#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "LightEngine/Fixture/Fixture.h"
#include "Utils/Storage/FragmentedStorage.h"

//
// Universe: one 512-channel DMX universe. It IS a FragmentedStorage of
// fixtures, so it owns the byte buffer and the placement/packing logic. On top
// of that it does two things the raw storage does not:
//   1. wires every placed fixture (and its parameters) to this universe's
//      buffer - the base storage assigns a start address but never a buffer;
//   2. keeps a name -> fixtures index for lookup.
//
// Use addFixture()/addFixtures() rather than the inherited add(); only the
// wrappers guarantee the buffer is wired.
//
namespace LightEngine::DMX
{
class Universe : public Utils::FragmentedStorage<LightEngine::Fixtures::Fixture, 512>
{
    using FixturePtr = std::shared_ptr<LightEngine::Fixtures::Fixture>;

    uint16_t m_id = 0;
    std::unordered_map<std::string, std::vector<FixturePtr>> m_byName;

    void wireBuffers(); // point every fixture (and its params) at our buffer
    void reindex();     // rebuild the name -> fixtures map

public:
    Universe() = default;
    explicit Universe(uint16_t id) : m_id(id) {}

    // Patch a fixture into the universe and wire it to the DMX buffer.
    // Returns the placed instance(s) so callers can track identity (e.g. FIDs).
    FixturePtr addFixture(const LightEngine::Fixtures::Fixture &fixture); // auto-place
    FixturePtr addFixture(const LightEngine::Fixtures::Fixture &fixture,
                          uint32_t start); // explicit address
    std::vector<FixturePtr> addFixtures(const LightEngine::Fixtures::Fixture &fixture,
                                        int count, uint32_t start);

    [[nodiscard]] uint16_t id() const { return m_id; }
    void setId(uint16_t id) { m_id = id; }

    [[nodiscard]] const std::list<FixturePtr> &fixtures() const
    {
        return m_fragments;
    }
    [[nodiscard]] FixturePtr at(std::size_t index) const;
    [[nodiscard]] const std::vector<FixturePtr> &
    byName(const std::string &name) const;
    [[nodiscard]] std::size_t fixtureCount() const { return getNumFragments(); }

    // Raw 512-byte DMX frame, for the output stage.
    [[nodiscard]] const std::array<uint8_t, 512> &buffer() const
    {
        return m_buffer;
    }

    // Zero the DMX values (keeps the patch). Called each frame before render.
    void blackout() { m_buffer.fill(0); }

    [[nodiscard]] std::string describe() const;

    // Pretty 16-column DMX dump: hex header, dashed separators, each channel
    // padded to 3 and coloured per owning fixture (unpatched channels dimmed).
    [[nodiscard]] std::string dump(int channels = 64) const;
};
} // namespace LightEngine::DMX
