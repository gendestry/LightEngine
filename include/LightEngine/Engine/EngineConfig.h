#pragma once
#include "Utils/Colors/Font.h"
#include "Utils/Network/Interfaces.h"
#include "Utils/Text/Stream.h"
#include "Utils/Traits/Stringify.h"
#include <string>

namespace LightEngine::Engine
{
struct Config : Utils::Traits::Stringify
{
    std::string version = "0.1";
    std::string name = "LightEngine";
    Utils::Network::IP ip = Utils::Network::Interfaces::primaryIP();
    bool output = true;

    [[nodiscard]] std::string toString() const override
    {
        // Utils::Text::Stream s;
        std::string ips = "";
        if (output)
        {
            ips = std::format("[{}]", Utils::String::colorWrap(
                                          Utils::Font::colorGreen, ip.str()));
        }
        std::string ret = std::format(
            "{}{} v{}", Utils::String::colorWrap(Utils::Font::colorBlue, name),
            ips, version);

        return ret;
    }
};
} // namespace LightEngine::Engine