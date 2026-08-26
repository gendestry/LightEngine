#include "LightEngine/Engine/Components/Patch.h"

using namespace LightEngine::Engine::Components;

LightEngine::DMX::UniversePatch &Patch::getUniverse(uint16_t universe)
{
    auto it = m_universes.find(universe);
    if (it == m_universes.end())
    {
        it = m_universes.try_emplace(universe, universe).first;
    }
    return it->second;
}

Patch::FixturePtr Patch::getFixture(uint64_t uid) const
{
    auto fixIt = m_fixtureByUID.find(uid);
    return fixIt != m_fixtureByUID.end() ? fixIt->second : nullptr;
}

std::optional<Patch::FixtureStatus> Patch::isPatched(uint32_t id) const
{
    auto statusIt = m_fixtureStatus.find(id);
    if (statusIt == m_fixtureStatus.end())
    {
        return std::nullopt;
    }

    return statusIt->second;
}

std::optional<Patch::FixPatch> Patch::patchInfo(uint32_t id) const
{
    auto status = isPatched(id);
    if (!status.has_value())
    {
        return std::nullopt;
    }

    auto uniIt = m_universes.find(status->universe);
    if (uniIt == m_universes.end())
    {
        return std::nullopt;
    }

    auto info = uniIt->second.getInfo(status->patchInfoID);
    if (!info.has_value())
    {
        return std::nullopt;
    }

    return FixPatch{status->universe, *info};
}

std::vector<Patch::FixturePtr> Patch::addFixtures(const Fixtures::Fixture &fixture, uint32_t amount)
{
    logger.debug("Adding {} fixtures", amount);
    std::vector<FixturePtr> fixtures;
    fixtures.reserve(amount);
    for (int i = 0; i < amount; i++)
    {
        auto fix = std::make_shared<Fixtures::Fixture>(fixture);
        m_fixtureByUID[m_fixCurrentID] = std::move(fix);
        fixtures.push_back(m_fixtureByUID[m_fixCurrentID++]);
    }

    logger.success("Added {} fixtures", amount);
    return fixtures;
}

bool Patch::removeFixture(uint32_t id)
{
    logger.debug("Removing fixture by id: {}", id);
    auto fixIt = m_fixtureByUID.find(id);
    if (fixIt != m_fixtureByUID.end())
    {
        unpatch(id);

        // drop it from the FID index, and drop the FID entirely once nothing carries it
        auto fidIt = m_fixtureByFIDs.find(fixIt->second->Fid());
        if (fidIt != m_fixtureByFIDs.end())
        {
            auto &uids = fidIt->second;
            uids.erase(std::remove(uids.begin(), uids.end(), id), uids.end());
            if (uids.empty())
            {
                m_fixtureByFIDs.erase(fidIt);
            }
        }

        m_fixtureByUID.erase(fixIt);
        logger.success("Fixture {} removed", id);
        return true;
    }

    logger.warn("Fixture doenst exist: {}", id);
    return false;
}

bool Patch::unpatch(uint32_t id)
{
    logger.debug("Unpatching fixture by id: {}", id);
    auto statusIt = m_fixtureStatus.find(id);
    if (statusIt == m_fixtureStatus.end())
    {
        logger.warn("Fixture is not patched: {}", id);
        return false;
    }

    const auto &status = statusIt->second;
    auto uniIt = m_universes.find(status.universe);
    if (uniIt == m_universes.end())
    {
        logger.error("Fixture {} references unknown universe {}", id, status.universe);
        m_fixtureStatus.erase(statusIt);
        return false;
    }

    uniIt->second.remove(status.patchInfoID);
    m_fixtureStatus.erase(statusIt);

    logger.success("Fixture {} unpatched from uni: {}", id, status.universe);
    return true;
}

bool Patch::patch(uint32_t fixUid, uint16_t universe, uint32_t addr)
{
    logger.debug("Patching fixture {}", fixUid);
    auto ispatched = patchInfo(fixUid);
    if (ispatched)
    {
        logger.error("Fixture {} already patched to uni: {}, addr: {}", fixUid, ispatched->universe, ispatched->info.start);
        return false;
    }
    auto fix = getFixture(fixUid);
    if (!fix)
    {
        logger.error("Fixture {} doesn't exist", fixUid);
        return false;
    }

    auto &f = *fix;
    auto &uni = getUniverse(universe);

    if (!uni.checkMultiple(addr, 1, f.size))
    {
        logger.error("Cannot patch fixture {} to uni:{} addr:{} - space occupied", fixUid,
                     universe, addr);
        return false;
    }

    auto &pinfo = uni.add(f.size, addr);
    m_fixtureStatus[fixUid] = FixtureStatus{universe, pinfo.id};

    f.setBuffer(uni.getRaw());
    f.setStart(pinfo.start);

    logger.success("Patched fixture {} to uni:{} addr:[{}-{}]", fixUid, universe, pinfo.start,
                   pinfo.start + pinfo.size - 1);
    return true;
}

bool Patch::patch(const std::vector<uint32_t> &fixUids, uint16_t universe, uint32_t addr)
{
    logger.debug("Patching {} fixtures to uni:{} addr:{}", fixUids.size(), universe, addr);

    if (fixUids.empty())
    {
        logger.error("Trying to patch 0 fixtures");
        return false;
    }

    // validate everything up front so we never leave a half-patched selection
    uint32_t total = 0;
    for (auto id : fixUids)
    {
        auto fix = getFixture(id);
        if (!fix)
        {
            logger.error("Fixture {} doesn't exist", id);
            return false;
        }

        if (auto info = patchInfo(id))
        {
            logger.error("Fixture {} already patched to uni: {}, addr: {}", id, info->universe,
                         info->info.start);
            return false;
        }

        total += fix->size;
    }

    auto &uni = getUniverse(universe);
    if (!uni.checkMultiple(addr, 1, total))
    {
        logger.error("Cannot patch {} fixtures to uni:{} addr:{} - space occupied",
                     fixUids.size(), universe, addr);
        return false;
    }

    uint32_t curr = addr;
    for (auto id : fixUids)
    {
        auto &f = *getFixture(id);
        auto &pinfo = uni.add(f.size, curr);

        m_fixtureStatus[id] = FixtureStatus{universe, pinfo.id};
        f.setBuffer(uni.getRaw());
        f.setStart(pinfo.start);

        curr += f.size;
    }

    logger.success("Patched {} fixtures to uni:{} addr:[{}-{}]", fixUids.size(), universe, addr,
                   curr - 1);
    return true;
}

uint32_t Patch::unpatch(const std::vector<uint32_t> &ids)
{
    logger.debug("Unpatching {} fixtures", ids.size());

    uint32_t count = 0;
    for (auto id : ids)
    {
        count += unpatch(id) ? 1 : 0;
    }

    logger.success("Unpatched {}/{} fixtures", count, ids.size());
    return count;
}

std::vector<uint16_t> Patch::patch(Fixtures::Fixture *fixtemplate, uint16_t universe,
                                   uint16_t amount, std::optional<uint32_t> start,
                                   std::optional<uint16_t> startFID)
{
    auto name = Utils::Font::format(Theme::name("'{}'"), fixtemplate->Name());
    logger.debug("Patching {} {} to uni:{} addr:{}", amount, name, universe, start.has_value() ? *start : 0);

    if (amount == 0)
    {
        logger.error("Trying to patch 0 fixtures");
        return {};
    }

    const auto &fixture = *fixtemplate;

    LightEngine::DMX::UniversePatch &uni = getUniverse(universe);

    uint32_t addr = start.has_value() ? *start : 0;
    auto placed = uni.addMultiple(fixture.size, addr, amount);

    if (placed.empty())
    {
        logger.error("Something went wrong");
        return {};
    }

    auto previd = m_fixCurrentID;
    auto fid = startFID ? *startFID : previd;
    auto fixtures = addFixtures(fixture, amount);

    std::vector<uint16_t> fids;
    fids.reserve(amount);

    for (int i = 0; i < amount; i++)
    {
        // logger.debug("FID would be: {}", fid);
        auto f = fixtures[i];
        auto &pinfo = placed[i].get();
        m_fixtureStatus[previd] = {universe, pinfo.id};
        m_fixtureByFIDs[static_cast<uint16_t>(fid)].push_back(previd); // fid -> uid

        f->setBuffer(uni.getRaw());
        f->setStart(pinfo.start);
        f->SetFid(static_cast<uint16_t>(fid));
        fids.push_back(f->Fid());

        previd++;
        fid++;
    }

    logger.success("Patched {} {} to uni:{} addr:[{}-{}]", amount, name, universe, start.has_value() ? *start : 0, placed.back().get().start + placed.back().get().size - 1);

    return fids;
}

std::vector<uint64_t> Patch::sortedByAddress() const
{
    // resolve each fixture's address once instead of inside the comparator, which would
    // re-walk the universe's fragment list O(n log n) times
    struct Key
    {
        uint64_t id;
        bool patched;
        uint16_t universe;
        uint32_t start;
    };

    std::vector<Key> keys;
    keys.reserve(m_fixtureByUID.size());

    for (const auto &[id, fix] : m_fixtureByUID)
    {
        if (auto info = patchInfo(id))
        {
            keys.push_back({id, true, info->universe, info->info.start});
        }
        else
        {
            keys.push_back({id, false, 0, 0});
        }
    }

    std::sort(keys.begin(), keys.end(), [](const Key &a, const Key &b)
              {
        // unpatched last, then by universe, then by address, then by uid
        if (a.patched != b.patched) { return a.patched; }
        if (!a.patched) { return a.id < b.id; }
        if (a.universe != b.universe) { return a.universe < b.universe; }
        if (a.start != b.start) { return a.start < b.start; }
        return a.id < b.id; });

    std::vector<uint64_t> ret;
    ret.reserve(keys.size());
    for (const auto &k : keys)
    {
        ret.push_back(k.id);
    }

    return ret;
}

void Patch::clearDMXBuffers()
{
    for (auto &[_, uni] : m_universes)
    {
        uni.clearBuffer();
    }
}

std::vector<Patch::FixturePtr> Patch::fixturesByFID(uint16_t fid) const
{
    auto it = m_fixtureByFIDs.find(fid);
    if (it == m_fixtureByFIDs.end())
        return {};

    std::vector<FixturePtr> ret;
    ret.reserve(it->second.size());
    for (auto uid : it->second)
        if (auto f = getFixture(uid))
            ret.push_back(f);
    return ret;
}

std::string Patch::toString() const
{
    std::string ret = "Patch\n";
    for (auto &[id, fix] : m_fixtureByUID)
    {
        std::string patched = "unpatched";

        auto statusIt = m_fixtureStatus.find(id);
        if (statusIt != m_fixtureStatus.end())
        {
            const auto &status = statusIt->second;

            auto uniIt = m_universes.find(status.universe);
            if (uniIt == m_universes.end())
            {
                patched = std::format("uni:{} <missing universe>", status.universe);
            }
            else if (auto info = uniIt->second.getInfo(status.patchInfoID))
            {
                patched = std::format("uni:{} addr:{}-{}", status.universe, info->start,
                                      info->start + info->size - 1);
            }
            else
            {
                patched = std::format("uni:{} <missing fragment {}>", status.universe,
                                      status.patchInfoID);
            }
        }

        const auto name = Utils::Font::format(Theme::name("'{}'"), fix->Name());
        ret += Utils::Font::format(Utils::Font::group("[", Theme::num("{}"), "] {} FID:",
                                                      Theme::num("{}"), " at {}\n"),
                                   id, name, fix->Fid(), patched);
    }

    return ret;
}

std::string Patch::uniDumpStr() const
{
    std::string ret;
    for (auto &[k, v] : m_universes)
    {
        ret += v.dump();
    }
    return ret;
}

void Patch::print() const
{
    auto l = Utils::String::split(toString(), "\n");
    for (auto &x : l)
    {
        logger.info("{}", x);
    }
}
