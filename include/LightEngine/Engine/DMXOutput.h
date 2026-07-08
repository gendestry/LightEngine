#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>

#include "Utils/Network/IP.h"
#include "Utils/Network/SACN.h"

#include "LightEngine/DMX/Universe.h"
#include "LightEngine/Engine/Patch.h"

//
// DMXOutput: the transmit stage. Holds one sACN sender per universe, lazily
// created, and streams universe buffers onto the network. It reads the live
// 512-byte frame straight out of each Universe (zero copy) and hands it to the
// sender. The Patch's dirty set decides which universes to send.
//
// TODO: this is sACN-only for now. A protocol interface (sACN / Art-Net / raw
// DMX) can be introduced later by turning m_senders into a map of an abstract
// sender type; the send()/update()/sendAll() surface stays the same.
//
namespace LightEngine::Engine
{
class DMXOutput
{
    Utils::Network::IP m_ip;               // local/source interface
    std::string m_sourceName = "Mixer";
    std::map<uint16_t, Utils::Network::SacnSender> m_senders; // by universe id

    Utils::Network::SacnSender &ensureSender(uint16_t universe);

public:
    DMXOutput() = default;
    explicit DMXOutput(const std::string &ip);

    void setIP(const std::string &ip);
    void setIP(const Utils::Network::IP &ip);
    void setSourceName(const std::string &name);

    // Send one universe's current frame.
    void send(const LightEngine::DMX::Universe &universe);

    // Send only the universes in `dirty` (looked up in `patch`).
    void update(const std::set<uint16_t> &dirty, Patch &patch);

    // Send every universe in the patch.
    void sendAll(Patch &patch);
};
} // namespace LightEngine::Engine
