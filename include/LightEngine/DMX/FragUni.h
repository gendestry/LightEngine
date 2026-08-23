//
// Created by Bobi on 9/21/25.
//

#pragma once
#include "Utils/Colors/HSV.h"
#include "Utils/Logging/Logger.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <format>
#include <iomanip>
#include <list>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <type_traits>
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

    void fillBytesPatched(uint32_t start, uint32_t end)
    {
        std::fill(m_bytesPatched.begin() + start, m_bytesPatched.begin() + end, m_fragIDCounter);
    }

    uint32_t numFragmentsBefore(uint32_t start)
    {
        uint32_t counter = 0U;
        uint32_t prev = 0U;

        for (uint32_t i = 0; i < start; i++)
        {
            auto v = m_bytesPatched[i];
            if (v != 0 && prev != v)
            {
                counter++;
                prev = v;
            }
        }

        return counter;
    }

    std::optional<uint32_t> findFirstEmpty(uint32_t size)
    {
        for (int i = 0; i < 512 - size; i++)
        {
            bool isEmpty = isFree(i);
            if (isEmpty)
            {
                uint32_t counter = 0;
                for (; counter < size; counter++)
                {
                    isEmpty = isFree(counter + i);
                    if (!isEmpty)
                    {
                        break;
                    }
                }

                if (counter == size)
                {
                    return i;
                }
            }
        }

        return std::nullopt;
    };

    bool isFree(uint32_t index) { return m_bytesPatched[index] == 0; }

    PatchInfo &addHelper(uint16_t size, uint32_t start)
    {
        PatchInfo info(start, size);
        fillBytesPatched(start, start + size);
        info.id = m_fragIDCounter++;

        auto n = numFragmentsBefore(start);
        auto it = std::next(m_fragments.begin(), n);
        auto inserted = m_fragments.insert(it, std::move(info));
        return *inserted;
    }

public:
    UniversePatch(uint16_t id) : m_id(id), logger(std::format("Universe {}", id)) {}
    bool checkMultiple(uint32_t start, uint32_t n, uint32_t size)
    {
        if (start + n * size > 512)
        {
            return false;
        }

        for (int i = 0; i < n * size; i++)
        {
            if (!isFree(start + i))
            {
                return false;
            }
        }

        return true;
    }

    PatchInfo &add(uint16_t size, uint32_t start)
    {
        bool isFilled = !isFree(start);
        for (uint32_t i = start + 1; (i < start + size) && !isFilled; i++)
        {
            isFilled = !isFree(i);
        }

        if (isFilled)
        {
            throw std::runtime_error("Fragment overlaps other segment");
        }

        return addHelper(size, start);
    }

    PatchInfo &add(uint16_t size)
    {
        auto v = findFirstEmpty(size);
        if (!v.has_value())
        {
            throw std::runtime_error("Cant find valid position");
        }

        auto start = v.value();
        return addHelper(size, start);
    }

    std::vector<std::reference_wrapper<PatchInfo>> addMultiple(int size, int start, int n)
    {
        if (!checkMultiple(start, n, size))
        {
            logger.error("Cannot find valid position for patching {} fixtures", n);
            return {};
            // throw std::runtime_error("Cant find valid position");
        }

        std::vector<std::reference_wrapper<PatchInfo>> ret;
        ret.reserve(n);

        uint32_t curr = start;
        for (int i = 0; i < n; i++)
        {
            ret.push_back(addHelper(size, curr));
            curr += size;
        }

        return ret;
    }

    // bool remove(const std::shared_ptr<T> &fragment)
    // {
    //     if (!fragment)
    //     {
    //         return false;
    //     }
    //     auto it = std::find(m_fragments.begin(), m_fragments.end(), fragment);
    //     if (it == m_fragments.end())
    //     {
    //         return false;
    //     }
    //     const uint32_t start = (*it)->start;
    //     const uint32_t size = (*it)->size; // Mark the fragment's space
    //     std::fill(
    //         m_bytesPatched.begin() + start, m_bytesPatched.begin() + start + size,
    //         0); // Remove the shared_ptr from the container. // If this is the last
    //             // shared_ptr owning the object, T is destroyed here.
    //     m_fragments.erase(it);
    //     return true;
    // }

    // void defragment()
    // {
    //     m_bytesPatched.fill(0);
    //     uint32_t curr = 0U;
    //     for (const auto &fragment : m_fragments)
    //     {
    //         uint32_t &start = fragment->start;
    //         const uint32_t &size = fragment->size;
    //         start = curr;
    //         curr += size;

    //         for (uint16_t i = start; i < start + size; i++)
    //         {
    //             m_bytesPatched[i] = fragment->id;
    //         }
    //     }
    // }

    // [[nodiscard]] size_t getNumFragments() const { return m_fragments.size(); }

    [[nodiscard]] uint16_t getID() const { return m_id; }

    [[nodiscard]] uint8_t *getRaw() { return m_buffer.data(); }

    [[nodiscard]] const std::array<uint8_t, 512> &buffer() const { return m_buffer; }

    [[nodiscard]] std::string fragmentsToString() const
    {
        std::stringstream ss;
        uint32_t curr = 0U;
        for (const auto &fragment : m_fragments)
        {
            const uint32_t &start = fragment.start;
            // col = nextColor(start);
            if (curr != start)
            {
                ss << std::format(
                    "[{:3}, {:3}] Free space [{} bytes]\n", curr, start - 1, (start - curr));
            }
            ss << std::format(
                "[{:3}, {:3}]\n", start, start + fragment.size - 1);
            curr = start + fragment.size;
        }

        if (curr != 512 - 1)
        {
            ss << std::format(
                "[{:3}, {:3}] Free space [{} bytes]\n", curr, 512 - 1, (512 - curr));
        }
        return ss.str();
    }

    std::string dump(int channels = 64) const
    {
        namespace F = Utils::Font;

        // map every channel to the index of the fixture that owns it (-1 =
        // unpatched)
        std::vector<int> owner(512, -1);
        int idx = 0;
        for (const auto &f : m_fragments)
        {
            for (uint32_t c = 0; c < f.size; ++c)
            {
                owner[f.start + c] = idx;
            }
            ++idx;
        }

        auto colorFor = [](int ownerIdx) -> std::string
        {
            if (ownerIdx < 0)
            {
                return F::colorDim;
            }
            // distinct hue per fixture
            Utils::Colors::HSV hsv(std::fmod(110.f + ownerIdx * 65.f, 360.f), 0.5f,
                                   0.9f);
            return F::colorByRGB(hsv.toRGB(), true);
        };

        const int cols = 16;
        std::ostringstream ss;
        ss << "Universe " << m_id << "  (" << m_fragments.size() << " fixtures)\n"
           << F::colorItalic;
        for (int i = 0; i < cols; ++i)
        {
            ss << "0x" << "0123456789ABCDEF"[i] << " ";
        }
        ss << F::colorReset << "\n";

        auto sep = [&]
        {
            for (int i = 0; i < cols; ++i)
                ss << "----";
            ss << "\n";
        };
        sep();

        for (int i = 0; i < channels; ++i)
        {
            if (i % cols == 0 && i != 0)
            {
                ss << "\n";
            }
            ss << colorFor(owner[i]) << std::setw(3) << int(m_buffer[i])
               << F::colorReset << " ";
        }
        ss << "\n";
        sep();
        return ss.str();
    }
};

} // namespace LightEngine::DMX
