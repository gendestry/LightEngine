#include "LightEngine/Engine/Engine.h"

#include <algorithm>

namespace LightEngine::Engine
{
void Engine::update(float dt)
{
    // advance the frame clock before composing, so every layer samples a
    // consistent "now" for the whole frame.
    m_time.dt = dt;
    m_time.now += dt;
    ++m_time.frame;

    m_patch.blackout();

    // 1. compose: layers write their contributions into the frame, composed in
    //    priority order (low -> high) so higher layers' LTP writes win.
    m_frame.clear();
    std::stable_sort(m_layers.begin(), m_layers.end(),
                     [](const Layer *a, const Layer *b)
                     { return a->priority() < b->priority(); });
    for (Layer *layer : m_layers)
    {
        if (layer->enabled())
        {
            layer->apply(m_frame, m_time);
        }
    }

    // 2. resolve: push each fixture's merged values into its DMX buffer.
    //    Fixtures are stateless sinks - anything not addressed this frame stays
    //    at the blackout value.
    for (const auto &[fid, values] : m_frame.all())
    {
        if (auto fixture = m_patch.getFixture(fid))
        {
            fixture->Resolve(values);
        }
    }

    // 3. output: continuous full-frame send, as a real sACN source does. Only
    //    once an IP has been configured (setIP), else this is a render-only run.
    if (m_outputEnabled)
    {
        m_output.sendAll(m_patch);
    }
    m_patch.clearDirty();
}
} // namespace LightEngine::Engine
