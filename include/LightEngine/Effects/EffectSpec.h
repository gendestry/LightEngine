#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "Utils/Colors/HSV.h"
#include "Utils/Colors/RGB.h"
#include "Utils/Math/Curve.h"

//
// EffectSpec: the serializable *description* of an effect - kind, target
// fixtures (by FID) and parameters. This is what a Cue/FxPreset stores; a
// running Effect is built from it by the EffectFactory, and an Effect reports
// its current Spec via Effect::spec(). One representation, both directions.
//
// Sample is the other half: what a running effect *produces* for one fixture on
// a given frame. Only the field matching the effect's Target is meaningful.
//
namespace LightEngine::Effects
{
enum class Kind : uint8_t
{
    DimmerChase, // travelling intensity wave (curve + bpm + spread)
    ColorFade,   // ping-pong between two colours
};

// Which programmable attribute an effect drives. Determines which Source slot
// it attaches to in the programmer, and which Sample field it fills.
enum class Target
{
    Intensity,
    Color,
};

[[nodiscard]] constexpr Target targetOf(Kind k)
{
    return k == Kind::ColorFade ? Target::Color : Target::Intensity;
}

struct Spec
{
    Kind kind = Kind::DimmerChase;
    std::vector<uint16_t> fids; // target fixtures (ordered)

    // parameters - only the ones relevant to `kind` are used
    Utils::Maths::Type curve = Utils::Maths::SINUSOID;
    float bpm = 60.0f;           // temporal rate (cycles per minute)
    float spread = 1.0f;         // spatial: full waves across the group
    float level = 1.0f;          // amplitude / fallback ceiling
    Utils::Colors::RGB colorA{}; // ColorFade / colour kinds
    Utils::Colors::RGB colorB{}; // ColorFade
};

// One effect's contribution for one fixture this frame. An unset field means
// "this effect does not drive that attribute".
struct Sample
{
    std::optional<float> level;              // Target::Intensity
    std::optional<Utils::Colors::HSV> color; // Target::Color
};
} // namespace LightEngine::Effects
