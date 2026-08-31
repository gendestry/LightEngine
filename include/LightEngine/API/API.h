// #pragma once
// #include "LightEngine/Engine/Engine.h"
// #include "Utils/Commands/Registry.h"
//
// namespace LightEngine
// {
// class Api
// {
//     Engine::Engine &m_engine;
//
//     Utils::Commands::Registry<Engine::Components::Patch>          m_patch{m_engine.patcher()};
//     Utils::Commands::Registry<Engine::Components::Programmer>     m_programmer{m_engine.programmer()};
//     Utils::Commands::Registry<Engine::Components::FixtureLibrary> m_fixlib{m_engine.library()};
//     Utils::Commands::Registry<Engine::Engine>              m_engineCmds{m_engine};
//
// public:
//     explicit Api(Engine::Engine &e) : m_engine(e) { /* register verbs */ }
//
//     // "patch.patch RGB 1 5", "prog.select 1-16", "fixlib.find RGB", "store group 1"
//     bool dispatch(const std::string &line);
// };
// }