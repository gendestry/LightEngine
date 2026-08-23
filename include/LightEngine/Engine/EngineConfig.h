#pragma once
#include "Utils/Colors/Font.h"
#include "Utils/Network/Interfaces.h"
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
        std::string ips = "";
        if (output)
        {
            ips = Utils::Font::format(Utils::Font::group("[", Theme::ok("{}"), "]"), ip.str());
        }
        else
        {
            ips = Utils::Font::format(Utils::Font::group("[", Theme::err("Disconnected"), "]"));
        }

        return Utils::Font::format(
            Utils::Font::group(
                Utils::Font::B(
                    Theme::pink("{}")),
                "{} v{}"),
            name, ips, version);
    }
};
} // namespace LightEngine::Engine