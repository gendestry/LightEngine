#include "LightEngine/Engine/Layer.h"

namespace LightEngine::Engine
{
Utils::Colors::HSV &ProgrammerLayer::ensureColor(uint16_t fid)
{
    FixtureValues &v = m_edits[fid];
    if (!v.color)
    {
        v.color = Utils::Colors::HSV{0.f, 0.f, 0.f};
    }
    return *v.color;
}

void ProgrammerLayer::select(const DMX::FixtureGroup &group)
{
    m_selection.clear();
    for (const auto &f : group.fixtures())
    {
        m_selection.push_back(f->Fid());
    }
}

void ProgrammerLayer::setColor(const Utils::Colors::HSV &hsv)
{
    for (uint16_t fid : m_selection)
    {
        ensureColor(fid) = hsv;
    }
}

void ProgrammerLayer::setHueSat(float h, float s)
{
    for (uint16_t fid : m_selection)
    {
        Utils::Colors::HSV &c = ensureColor(fid);
        c.h = h;
        c.s = s;
    }
}

void ProgrammerLayer::setIntensity(float v)
{
    for (uint16_t fid : m_selection)
    {
        ensureColor(fid).v = v;
    }
}

void ProgrammerLayer::setIntensityRamp(float a, float b)
{
    const std::size_t n = m_selection.size();
    for (std::size_t i = 0; i < n; ++i)
    {
        float t = n <= 1 ? 0.f : float(i) / float(n - 1);
        ensureColor(m_selection[i]).v = a + (b - a) * t;
    }
}

void ProgrammerLayer::apply(Frame &frame, const TimeContext &)
{
    // Programmer edits are absolute overrides -> LTP.
    for (const auto &[fid, values] : m_edits)
    {
        frame.contribute(fid, values, MergePolicy::LTP);
    }
}
} // namespace LightEngine::Engine
