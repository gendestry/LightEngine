#pragma once
#include "LightEngine/Fixture/Parameter.h"
#include "LightEngine/GDTF/LogicalChannel.h"
#include "Utils/Colors/Colors.h"

//
// ColorCell: a view over the color parameters of ONE emitter/pixel (the params
// that share a cellIndex). It owns no bytes and no parameters - just pointers
// into the Fixture's parameter list plus this emitter's HSV state.
//
// It is NOT a Parameter (a Parameter is one channel; a cell bundles several).
// It is the single place the HSV -> RGB + virtual-dimmer rule lives:
//   - if the cell has a real DIMMER param: V goes there, color written at full;
//   - otherwise V is folded into the color (virtual dimmer).
//
namespace LightEngine::Fixtures
{
class ColorCell
{
    Parameter *m_r = nullptr;
    Parameter *m_g = nullptr;
    Parameter *m_b = nullptr;
    Parameter *m_w = nullptr;      // nullptr if the cell has no white
    Parameter *m_dimmer = nullptr; // nullptr => virtual dimmer

    Utils::Colors::HSV m_hsv{0.f, 0.f, 0.f};

public:
    ColorCell() = default;

    // ---- wiring (called by the Fixture while grouping params by cellIndex) ----
    void SetComponent(GDTF::Attribute attr, Parameter *p)
    {
        switch (attr)
        {
        case GDTF::Attribute::COLOR_R: m_r = p; break;
        case GDTF::Attribute::COLOR_G: m_g = p; break;
        case GDTF::Attribute::COLOR_B: m_b = p; break;
        case GDTF::Attribute::COLOR_W: m_w = p; break;
        case GDTF::Attribute::DIMMER: m_dimmer = p; break;
        default: break;
        }
    }

    bool IsEmpty() const { return !m_r && !m_g && !m_b; }

    // ---- editing ----
    void SetColor(const Utils::Colors::HSV &hsv) { m_hsv = hsv; }
    void SetHueSat(float h, float s)
    {
        m_hsv.h = h;
        m_hsv.s = s;
    }
    void SetIntensity(float v) { m_hsv.v = v; } // intensity == V component

    // ---- frame resolve: HSV state -> DMX bytes via the parameters ----
    void Resolve()
    {
        Utils::Colors::HSV out = m_hsv;
        if (m_dimmer)
        {
            m_dimmer->Write(m_hsv.v); // real dimmer takes V
            out.v = 1.f;              // color at full brightness
        }

        // hsvToRgb gives 8-bit components; Parameter::Write wants 0..1
        Utils::Colors::RGB rgb = Utils::Colors::hsvToRgb(out);
        if (m_r) m_r->Write(rgb.r / 255.f);
        if (m_g) m_g->Write(rgb.g / 255.f);
        if (m_b) m_b->Write(rgb.b / 255.f);

        // naive RGBW: the white channel carries the achromatic part - the less
        // saturated the color, the more white. Scaled by the effective V.
        if (m_w) m_w->Write((1.f - out.s) * out.v);
    }
};
} // namespace LightEngine::Fixtures
