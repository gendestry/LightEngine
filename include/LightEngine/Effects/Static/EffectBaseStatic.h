#pragma once
#include "LightEngine/Effects/EffectBase.h"

namespace LightEngine::Effects
{
class EffectStatic : public EffectBase
{
public:
    // EffectStatic() : EffectBase(EffectType::STATIC) {}
    EffectStatic() : EffectBase() {}
    EffectStatic(
        const std::vector<std::pair<uint16_t, Engine::FixtureValues>> &vals)
        : EffectBase()
    {
        setCache(vals);
    }

    virtual void setCache(
        const std::vector<std::pair<uint16_t, Engine::FixtureValues>> &vals)
    {
        m_cache = vals;
    }

    EFFECT_TYPE(STATIC);
};
} // namespace LightEngine::Effects