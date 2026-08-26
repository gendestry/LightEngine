#pragma once
#include <cstdint>
#include <string>

#include "Utils/Colors/ColorFormatter.h"
#include "Utils/Colors/Theme.h"
#include "Utils/Traits/Stringify.h"

namespace LightEngine::Engine
{

class PoolObject : public Utils::Traits::Stringify
{
    uint32_t m_number = 0;
    std::string m_name;

protected:
    void setNumber(uint32_t n) { m_number = n; }
    template <class>
    friend class Pool;

public:
    virtual ~PoolObject() = default;
    uint32_t number() const { return m_number; }
    const std::string &name() const { return m_name; }
    void setName(std::string n) { m_name = std::move(n); }

    // One-line identity: "#3 Front wash" (number dim, name bold). Subclasses
    // override to append their payload, and usually prefix this base version.
    [[nodiscard]] virtual std::string toString() const = 0;
    // {
    //     // m_name is user data and may contain braces, so it goes in as a "{}"
    //     // argument rather than being baked into the format string.
    //     const auto nameExpr =
    //         m_name.empty() ? Theme::dim("Unnamed") : Theme::txt("{}");

    //     // Uncoloured outer scope: every child restores to plain text, so the
    //     // brackets and separators stay default-coloured.
    //     return Utils::Font::format(Utils::Font::group(Theme::lbl("Preset "), "[", Theme::num("{}"), "]: ", nameExpr),
    //                                m_number, m_name);
    // }
};

} // namespace LightEngine::Engine