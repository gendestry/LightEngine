#include "LightEngine/Output/DMXOutput.h"

namespace LightEngine::Output
{
DMXOutput::DMXOutput(const std::string &ip) { m_ip.setIP(ip); }

Utils::Network::SacnSender &DMXOutput::ensureSender(uint16_t universe)
{
    auto it = m_senders.find(universe);
    if (it == m_senders.end())
    {
        it = m_senders.try_emplace(universe).first;
        it->second.begin(static_cast<uint8_t>(universe), m_ip);
        it->second.setSourceName(m_sourceName);
    }
    return it->second;
}

void DMXOutput::setIP(const std::string &ip)
{
    m_ip.setIP(ip);
    for (auto &[universe, sender] : m_senders)
        sender.setIP(m_ip);
}

void DMXOutput::setIP(const Utils::Network::IP &ip)
{
    m_ip = ip;
    for (auto &[universe, sender] : m_senders)
        sender.setIP(m_ip);
}

void DMXOutput::setSourceName(const std::string &name)
{
    m_sourceName = name;
    for (auto &[universe, sender] : m_senders)
        sender.setSourceName(name);
}

void DMXOutput::send(const LightEngine::DMX::UniversePatch &universe)
{
    ensureSender(universe.getID()).send(universe.buffer());
}

void DMXOutput::update(const std::set<uint16_t> &dirty, Engine::Components::Patch &patch)
{
    // for (uint16_t universe : dirty)
    //     if (const LightEngine::DMX::Universe *uni = patch.getUniverse(universe))
    //         send(*uni);
}

void DMXOutput::sendAll(Engine::Components::Patch &patch)
{
    for (const auto &[id, uni] : patch.universes())
        send(uni);
}
} // namespace LightEngine::Output
