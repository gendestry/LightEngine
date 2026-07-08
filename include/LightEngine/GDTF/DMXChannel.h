#pragma once
#include <cstdint>

namespace LightEngine::GDTF
{

struct DMXChannel
{
    enum class Resolution : uint16_t
    {
        Bit8 = 255,
        Bit16 = 65535
    };

    uint16_t address = 0;
    Resolution res = Resolution::Bit8;

    uint16_t MaxDMX() const
    {
        if (res == Resolution::Bit8)
        {
            return 255U;
        }

        return 65535U;
    }
};
} // namespace LightEngine::GDTF
