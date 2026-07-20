#pragma once

#include <cstdint>
#include <vector>

#include "Utils/Colors/RGB.h"
#include "Utils/Math/Curve.h"

//
// EffectSpec: the serializable *description* of an effect - kind, target
// fixtures (by FID) and parameters. This is what a Cue stores; a running Effect
// is built from it by the EffectFactory, and an Effect can produce its current
// Spec via Effect::spec(). One representation, both directions.
//
namespace LightEngine::Effects
{
    enum class Kind : uint8_t
    {
        ColorEffect,
        DimmerEffect,
        ColorFade,
        DimmerChase,
    };

    struct Spec
    {
        Kind                  kind = Kind::DimmerChase;
        std::vector<uint16_t> fids;                            // target fixtures

        // parameters (only the ones relevant to `kind` are used)
        Utils::Maths::Type    curve  = Utils::Maths::SINUSOID;
        float                 bpm    = 60.0f;
        float                 spread = 1.0f;
        float                 level  = 1.0f;                   // DimmerEffect
        Utils::Colors::RGB    colorA{};                        // ColorEffect / ColorFade
        Utils::Colors::RGB    colorB{};                        // ColorFade
    };
}
