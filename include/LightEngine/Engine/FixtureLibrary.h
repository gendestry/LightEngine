#pragma once
#include "LightEngine/Fixture/Fixture.h"
#include <string>
#include <unordered_map>

namespace LightEngine::Engine
{
class FixtureLibrary
{
    std::unordered_map<std::string, Fixtures::Fixture> m_data;

public:
    void add(const Fixtures::Fixture &fix)
    {
        m_data[fix.Name()] = std::move(fix);
    }

    Fixtures::Fixture get(const std::string name) { return m_data[name]; }
};
} // namespace LightEngine::Engine