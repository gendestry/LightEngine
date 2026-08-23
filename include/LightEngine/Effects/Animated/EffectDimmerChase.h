// #pragma once
// #include "LightEngine/Effects/EffectBase.h"

// namespace LightEngine::Effects
// {
// // A travelling intensity wave across the selection, shaped by a curve.
// //   bpm        - temporal rate: one full cycle of the curve per beat.
// //   spread     - spatial distribution: how many full waves span the selection.
// //   resolution - samples per cycle. This is also the effect's step count, so it
// //                doubles as the scheduling grain: the chase only recomputes on
// //                curve-sample boundaries, not on every frame.
// class DimmerChase : public EffectAnimated
// {
//     Utils::Maths::Type m_type;

// public:
//     DimmerChase(Utils::Maths::Type type = Utils::Maths::SINUSOID,
//                 float bpm = 60.0f, float spread = 1.0f,
//                 uint16_t resolution = 128);

//     void setCurve(Utils::Maths::Type type);
//     [[nodiscard]] Utils::Maths::Type curve() const { return m_type; }

//     EFFECT_CATEGORY(DIMMER);

// protected:
//     void recompute(const Engine::TimeContext &t,
//                    const Utils::Maths::Interval &g) override;
// };
// } // namespace LightEngine::Effects
