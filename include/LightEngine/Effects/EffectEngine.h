#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Effects/Effect.h"
#include "LightEngine/Effects/EffectFactory.h"
#include "LightEngine/Effects/EffectSpec.h"

//
// EffectEngine: the sole owner of live Effect objects. In the pull model it has
// NO per-frame tick - the ProgrammerLayer queries each effect's valueFor()
// while composing the Frame. This class is purely lifetime management, so that
// several attribute-sources can share one effect (e.g. a single DimmerChase
// referenced by every fid's intensity slot in its group).
//
namespace LightEngine::Effects
{
class EffectEngine
{
    std::vector<std::unique_ptr<Effect>> m_effects;

public:
    // Build an effect from a Spec bound to `group`, store it, and hand back a
    // non-owning pointer for the programmer's attribute slots to reference.
    Effect *build(const Spec &spec, DMX::FixtureGroup group)
    {
        auto fx = EffectFactory::build(spec, std::move(group));
        if (!fx)
            return nullptr;
        Effect *raw = fx.get();
        m_effects.push_back(std::move(fx));
        return raw;
    }

    Effect *add(std::unique_ptr<Effect> fx)
    {
        Effect *raw = fx.get();
        m_effects.push_back(std::move(fx));
        return raw;
    }

    void remove(Effect *fx)
    {
        std::erase_if(m_effects,
                      [&](const std::unique_ptr<Effect> &e) { return e.get() == fx; });
    }
    void clear() { m_effects.clear(); }

    [[nodiscard]] std::size_t size() const { return m_effects.size(); }
    [[nodiscard]] const std::vector<std::unique_ptr<Effect>> &effects() const
    {
        return m_effects;
    }
};
} // namespace LightEngine::Effects
