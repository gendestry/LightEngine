// #include "LightEngine/Effects/StaticIntensity.h"

// namespace LightEngine::Effects
// {

// void StaticIntensity::apply(Engine::Frame &frame, const Engine::TimeContext
// &)
// {
//     Engine::FixtureValues v;
//     v.intensity = m_level;
//     for (uint16_t fid : m_group.fids())
//         frame.contribute(fid, v, Engine::MergePolicy::LTP); // absolute set
// }

// Spec StaticIntensity::spec() const
// {
//     Spec s;
//     s.kind = Kind::DimmerEffect;
//     s.fids = targetFids();
//     s.level = m_level;
//     return s;
// }
// } // namespace LightEngine::Effects