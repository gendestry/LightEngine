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

void ProgrammerLayer::setColor(const Utils::Colors::HSV &hsv)
{
    // Color and intensity are independent: setColor touches only hue/sat and
    // never the dimmer. If intensity is 0 the fixture stays dark regardless.
    for (const auto &f : m_selection.fixtures())
    {
        Utils::Colors::HSV &c = ensureColor(f->Fid());
        c.h = hsv.h;
        c.s = hsv.s;
    }
}

void ProgrammerLayer::setColor(const Utils::Colors::RGB &rgb)
{
    setColor(Utils::Colors::rgbToHsv(rgb));
}

void ProgrammerLayer::setHueSat(float h, float s)
{
    for (const auto &f : m_selection.fixtures())
    {
        Utils::Colors::HSV &c = ensureColor(f->Fid());
        c.h = h;
        c.s = s;
    }
}

void ProgrammerLayer::setIntensity(float v)
{
    for (const auto &f : m_selection.fixtures())
    {
        m_edits[f->Fid()].intensity = v;
    }
}

void ProgrammerLayer::setIntensityRamp(float a, float b)
{
    const auto &fixtures = m_selection.fixtures();
    const std::size_t n = fixtures.size();
    for (std::size_t i = 0; i < n; ++i)
    {
        float t = n <= 1 ? 0.f : float(i) / float(n - 1);
        m_edits[fixtures[i]->Fid()].intensity = a + (b - a) * t;
    }
}

void ProgrammerLayer::fanColor(float hueA, float hueB, float sat)
{
    const auto &fixtures = m_selection.fixtures();
    const std::size_t n = fixtures.size();
    for (std::size_t i = 0; i < n; ++i)
    {
        float t = n <= 1 ? 0.f : float(i) / float(n - 1);
        Utils::Colors::HSV &c = ensureColor(fixtures[i]->Fid());
        c.h = hueA + (hueB - hueA) * t;
        c.s = sat;
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
