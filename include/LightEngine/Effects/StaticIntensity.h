// #pragma once

// #include "LightEngine/Effects/Effect.h"
// #include <utility>

// //
// // Concrete effects. Static ones ignore time; dynamic ones animate off the
// // TimeContext. Per-fixture effects use the fixture's index within the group.
// // Each effect owns its group (passed by value), contributes into the frame
// and
// // can report its Spec.
// //
// namespace LightEngine::Effects
// {
// class StaticIntensity : public Effect
// {
//     float m_level;

// public:
//     StaticIntensity(DMX::FixtureGroup g, float level)
//         : Effect(std::move(g)), m_level(level)
//     {
//     }
//     void setLevel(float level) { m_level = level; }
//     void apply(Engine::Frame &frame, const Engine::TimeContext &t) override;
//     [[nodiscard]] Spec spec() const override;
// };
// } // namespace LightEngine::Effects