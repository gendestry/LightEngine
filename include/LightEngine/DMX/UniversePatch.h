//
// Created by Bobi on 9/21/25.
//

#pragma once
#include "Utils/Logging/Logger.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <list>
#include <optional>
#include <vector>

namespace LightEngine::DMX
{

class PatchInfo
{
public:
    uint32_t start = 0U;
    uint32_t size = 0U;
    uint32_t id = 0U;

    PatchInfo() = default;
    explicit PatchInfo(uint32_t size) : size(size) {}
    explicit PatchInfo(uint32_t start, uint32_t size) : start(start), size(size) {}

    PatchInfo(const PatchInfo &other)
    {
        size = other.size;
        start = other.start;
    }

    PatchInfo(PatchInfo &&other) = default;
    PatchInfo &operator=(const PatchInfo &other) = default;
    PatchInfo &operator=(PatchInfo &&other) = default;

    void setStart(uint32_t st) { start = st; };
};

class UniversePatch
{
private:
    std::list<PatchInfo> m_fragments;
    std::array<uint8_t, 512> m_buffer = {0U};
    std::array<uint16_t, 512> m_bytesPatched = {0U};
    uint16_t m_id = 0;
    uint16_t m_fragIDCounter = 1U;

    Utils::Logger logger;

    void fillBytesPatched(uint32_t start, uint32_t end);

    uint32_t numFragmentsBefore(uint32_t start);
    std::optional<uint32_t> findFirstEmpty(uint32_t size);

    bool isFree(uint32_t index);
    PatchInfo &addHelper(uint16_t size, uint32_t start);

public:
    UniversePatch(uint16_t id);

    bool checkMultiple(uint32_t start, uint32_t n, uint32_t size);

    PatchInfo &add(uint16_t size, uint32_t start);
    PatchInfo &add(uint16_t size);

    std::vector<std::reference_wrapper<PatchInfo>> addMultiple(int size, int start, int n);

    bool remove(uint32_t fragID);
    void clearBuffer() { m_buffer.fill(0); }

    [[nodiscard]] std::optional<PatchInfo> getInfo(uint32_t id) const
    {
        auto it = std::find_if(m_fragments.begin(), m_fragments.end(), [&](const auto &f)
                               { return f.id == id; });

        if (it == m_fragments.end())
        {
            return std::nullopt;
        }

        return *it;
    }

    [[nodiscard]] inline uint16_t getID() const { return m_id; }

    [[nodiscard]] inline uint8_t *getRaw() { return m_buffer.data(); }
    [[nodiscard]] inline const std::array<uint8_t, 512> &buffer() const { return m_buffer; }

    [[nodiscard]] std::string fragmentsToString() const;
    [[nodiscard]] std::string dump(uint16_t channels = 64) const;
};

} // namespace LightEngine::DMX
