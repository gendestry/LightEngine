#include "LightEngine/DMX/UniversePatch.h"
#include "Utils/Colors/HSV.h"
#include <cmath>
#include <format>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace LightEngine::DMX;

void UniversePatch::fillBytesPatched(uint32_t start, uint32_t end)
{
    std::fill(m_bytesPatched.begin() + start, m_bytesPatched.begin() + end, m_fragIDCounter);
}

uint32_t UniversePatch::numFragmentsBefore(uint32_t start)
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

std::optional<uint32_t> UniversePatch::findFirstEmpty(uint32_t size)
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

bool UniversePatch::isFree(uint32_t index) { return m_bytesPatched[index] == 0; }

PatchInfo &UniversePatch::addHelper(uint16_t size, uint32_t start)
{
    PatchInfo info(start, size);
    fillBytesPatched(start, start + size);
    info.id = m_fragIDCounter++;

    auto n = numFragmentsBefore(start);
    auto it = std::next(m_fragments.begin(), n);
    auto inserted = m_fragments.insert(it, std::move(info));
    return *inserted;
}

UniversePatch::UniversePatch(uint16_t id) : m_id(id), logger(std::format("Universe {}", id))
{
}

bool UniversePatch::checkMultiple(uint32_t start, uint32_t n, uint32_t size)
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

PatchInfo &UniversePatch::add(uint16_t size, uint32_t start)
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

PatchInfo &UniversePatch::add(uint16_t size)
{
    auto v = findFirstEmpty(size);
    if (!v.has_value())
    {
        throw std::runtime_error("Cant find valid position");
    }

    auto start = v.value();
    return addHelper(size, start);
}

std::vector<std::reference_wrapper<PatchInfo>> UniversePatch::addMultiple(int size, int start, int n)
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

bool UniversePatch::remove(uint32_t fragID)
{
    auto it = std::find_if(m_fragments.begin(), m_fragments.end(),
                           [&](const auto &f)
                           { return f.id == fragID; });
    if (it == m_fragments.end())
    {
        return false;
    }

    const uint32_t start = it->start;
    const uint32_t size = it->size; // Mark the fragment's space
    std::fill(m_bytesPatched.begin() + start, m_bytesPatched.begin() + start + size, 0);
    m_fragments.erase(it);
    return true;
}

std::string UniversePatch::fragmentsToString() const
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

std::string UniversePatch::dump(uint16_t channels) const
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