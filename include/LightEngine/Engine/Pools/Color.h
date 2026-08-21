#pragma once
#include "LightEngine/Effects/Static/EffectColor.h"
#include "PoolObject.h"
#include <memory>

namespace LightEngine::Engine::Pools
{

class Color : public PoolObject
{
    std::shared_ptr<Effects::StaticColor> m_effect;

public:
    Color(std::shared_ptr<Effects::StaticColor> effect)
        : m_effect(std::move(effect))
    {
    }

    [[nodiscard]] std::string toString() const override
    {
        const auto nameExpr =
            name().empty() ? Theme::dim("Unnamed") : Theme::txt("{}");

        return Utils::Font::format(Utils::Font::group(Theme::lbl("Color Preset "), "[", Theme::num("{}"), "]: ", nameExpr),
                                   number(), name());
    }
};
} // namespace LightEngine::Engine::Pools