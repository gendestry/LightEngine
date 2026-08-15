// #pragma once

// #include "Utils/Colors/Gradient.h"

// #include "LightEngine/Effect/EffectBase.h"

// //
// // Concrete effects. Static ones ignore time; dynamic ones animate off the
// // TimeContext. Per-fixture effects use the fixture's index within the group.
// // Each effect owns its group (passed by value), contributes into the frame
// and
//     // can report its Spec.
//     //
//     namespace LightEngine::Effects
// {
//     // Holds the group at a constant colour (a "set colour" is just this).
//     class StaticGradient : public EffectStatic
//     {
//         Utils::Colors::Gradient gradient;

//     public:
//         StaticGradient(Utils::Colors::RGB color) : m_color(color) {}
//         void setColor(Utils::Colors::RGB color)
//         {
//             m_color = color;
//             markDirty();
//         }

//     protected:
//         void recompute(const Engine::TimeContext &t,
//                        const DMX::FixtureGroup &group) override;

//     public:
//         EFFECT_CATEGORY(COLOR);

//         // std::string describe() const override
//         // {
//         //     auto &c = m_color;
//         //     return std::format("Set color to {} {} {}", c.r, c.g, c.b);
//         // };
//         // [[nodiscard]] Spec spec() const override;
//     };

// } // namespace LightEngine::Effects