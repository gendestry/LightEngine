#pragma once
#include "LightEngine/Effects/EffectGroup.h"
#include <memory>
#include <vector>

namespace LightEngine::Effects
{
class Engine
{
    using EffectGroupPtr = std::shared_ptr<EffectGroup>;
    std::vector<EffectGroupPtr> m_effects;

public:
    // EffectGroupPtr add(s)
};
} // namespace LightEngine::Effects