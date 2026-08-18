#pragma once
#include <cstdint>
#include <string>

#include "Utils/Colors/Font.h"

namespace LightEngine::Engine
{

class PoolObject
{
    uint32_t m_number = 0;
    std::string m_name;

protected:
    void setNumber(uint32_t n) { m_number = n; }
    template <class> friend class Pool;

public:
    virtual ~PoolObject() = default;
    uint32_t number() const { return m_number; }
    const std::string &name() const { return m_name; }
    void setName(std::string n) { m_name = std::move(n); }

    // One-line identity: "#3 Front wash" (number dim, name bold). Subclasses
    // override to append their payload, and usually prefix this base version.
    virtual std::string describe() const
    {
        return Utils::Font::colorDim + "#" + std::to_string(m_number) + " " +
               Utils::Font::reset + Utils::Font::bold +
               (m_name.empty() ? Utils::Font::colorDim + "(unnamed)" : m_name) +
               Utils::Font::reset;
    }
};

} // namespace LightEngine::Engine