#pragma once
#include "LightEngine/Effects/EffectBase.h"

namespace LightEngine::Effects
{
class EffectStatic : public EffectBase
{
public:
    EffectStatic() = default;

    EFFECT_TYPE(STATIC);
};
} // namespace LightEngine::Effects