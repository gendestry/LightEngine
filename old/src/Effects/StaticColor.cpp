// #include "LightEngine/Effects/StaticColor.h"

// #include "Utils/Colors/Colors.h"

// namespace LightEngine::Effects
// {

// void StaticColor::apply(Engine::Frame &frame, const Engine::TimeContext &)
// {
//     Engine::FixtureValues v;
//     v.color = Utils::Colors::rgbToHsv(m_color); // frame stores colour as HSV
//     for (uint16_t fid : m_group.fids())
//         frame.contribute(fid, v,
//                          Engine::MergePolicy::LTP); // colour: latest wins
// }

// Spec StaticColor::spec() const
// {
//     Spec s;
//     s.kind = Kind::ColorEffect;
//     s.fids = targetFids();
//     s.colorA = m_color;
//     return s;
// }

// void StaticColorGrad::apply(Engine::Frame &frame, const Engine::TimeContext
// &)
// {
//     const auto &fids = m_group.fids();
//     // one colour stop per fixture, evenly ramped start -> end across the
//     group const std::vector<Utils::Colors::RGB> colors =
//     Utils::Colors::gradient(
//         m_startColor, m_endColor, static_cast<uint16_t>(fids.size()));

//     for (std::size_t i = 0; i < fids.size(); ++i)
//     {
//         Engine::FixtureValues v;
//         v.color = Utils::Colors::rgbToHsv(colors[i]); // frame stores colour
//         as HSV frame.contribute(fids[i], v, Engine::MergePolicy::LTP);
//     }
// }

// Spec StaticColorGrad::spec() const
// {
//     Spec s;
//     s.kind = Kind::ColorFade;
//     s.fids = targetFids();
//     s.colorA = m_startColor;
//     s.colorB = m_endColor;
//     return s;
// }
// } // namespace LightEngine::Effects