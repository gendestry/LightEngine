// #pragma once

// #include <utility>

// #include "Utils/Colors/RGB.h"
// // #include ""

// #include "LightEngine/Effects/Effect.h"

// //
// // Concrete effects. Static ones ignore time; dynamic ones animate off the
// // TimeContext. Per-fixture effects use the fixture's index within the group.
// // Each effect owns its group (passed by value), contributes into the frame
// and
// // can report its Spec.
// //
// namespace LightEngine::Effects
// {
// // Holds the group at a constant colour (a "set colour" is just this).
// class StaticColor : public Effect
// {
//     Utils::Colors::RGB m_color;

// public:
//     StaticColor(DMX::FixtureGroup g, Utils::Colors::RGB color)
//         : Effect(std::move(g)), m_color(color)
//     {
//     }
//     void setColor(Utils::Colors::RGB color) { m_color = color; }
//     void apply(Engine::Frame &frame, const Engine::TimeContext &t) override;
//     [[nodiscard]] Spec spec() const override;
// };

// class StaticColorGrad : public Effect
// {
//     Utils::Colors::RGB m_startColor;
//     Utils::Colors::RGB m_endColor;

// public:
//     StaticColorGrad(DMX::FixtureGroup g, const Utils::Colors::RGB &color,
//                     const Utils::Colors::RGB &color2)
//         : Effect(std::move(g)), m_startColor(color), m_endColor(color2)
//     {
//     }
//     void setGradient(const Utils::Colors::RGB &color,
//                      const Utils::Colors::RGB &color2)
//     {
//         m_startColor = color;
//         m_endColor = color2;
//     }
//     void apply(Engine::Frame &frame, const Engine::TimeContext &t) override;
//     [[nodiscard]] Spec spec() const override;
// };
// } // namespace LightEngine::Effects