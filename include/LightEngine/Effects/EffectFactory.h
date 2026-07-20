#pragma once

#include <memory>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Effects/Effect.h"
#include "LightEngine/Effects/EffectSpec.h"

//
// EffectFactory: builds a running Effect from a Spec, bound to the given group.
// This is the single construction path - used both when adding an effect live
// in the programmer and when recalling one from a cue.
//
namespace LightEngine::Effects::EffectFactory
{
    std::unique_ptr<Effect> build(const Spec& spec, DMX::FixtureGroup group);
}
