#pragma once
#include "LightEngine/Effects/Static/EffectColor.h"
#include "PoolObject.h"
#include <memory>

namespace LightEngine::Engine::Pools
{

class Color : public Engine::PoolObject
{
    std::shared_ptr<Effects::StaticColor> m_effect;

public:
    Color(std::shared_ptr<Effects::StaticColor> effect)
        : m_effect(std::move(effect))
    {
    }

    std::string describe() const override
    {
        return "Color " + std::to_string(number()) + " \"" + name() + "\" ";
    }
};
} // namespace LightEngine::Engine::Pools