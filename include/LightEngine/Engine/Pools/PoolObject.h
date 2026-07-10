#pragma once
#include <cstdint>
#include <string>

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
    virtual std::string describe() const = 0;
};

} // namespace LightEngine::Engine