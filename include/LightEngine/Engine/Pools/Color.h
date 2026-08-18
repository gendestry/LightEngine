#pragma once
#include "LightEngine/Effects/EffectGroup.h"
#include "PoolObject.h"
#include <list>
#include <memory>

namespace LightEngine::Engine::Pools
{

class Color : public Engine::PoolObject
{
    std::list<Effects::EffectWrapper> m_colors;

public:
    Color(const std::list<Effects::EffectWrapper> &staticColors)
        : m_colors(staticColors)
    {
    }

    ~Color() { m_colors.clear(); }

    std::list<Effects::EffectWrapper> get() { return m_colors; }

    std::string describe() const override
    {
        return "Color " + std::to_string(number()) + " \"" + name() + "\" ";
    }
};
} // namespace LightEngine::Engine::Pools