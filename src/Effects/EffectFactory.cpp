// #include "LightEngine/Effects/EffectFactory.h"

// #include <utility>

// #include "LightEngine/Effects/Effects.h"

// namespace LightEngine::Effects::EffectFactory
// {
//     std::unique_ptr<Effect> build(const Spec& spec, DMX::FixtureGroup group)
//     {
//         switch (spec.kind)
//         {
//             case Kind::ColorEffect:
//                 return std::make_unique<ColorEffect>(std::move(group),
//                 spec.colorA);
//             case Kind::DimmerEffect:
//                 return std::make_unique<DimmerEffect>(std::move(group),
//                 spec.level);
//             case Kind::ColorFade:
//                 return std::make_unique<ColorFade>(std::move(group),
//                 spec.colorA, spec.colorB, spec.bpm / 60.0f);
//             case Kind::DimmerChase:
//                 return std::make_unique<DimmerChase>(std::move(group),
//                 spec.curve, spec.bpm, spec.spread);
//         }
//         return nullptr;
//     }
// }
