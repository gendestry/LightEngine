#pragma once
#include <cstdint>
#include <map>
#include <string>

#include "LightEngine/DMX/FixtureGroup.h"
#include "LightEngine/Engine/Layer.h"
#include "LightEngine/Engine/Pools/PoolObject.h"

//
// Preset: a numbered, per-fixture value snapshot. The map keys ARE the list of
// FIDs the preset applies to, so recalling onto a fixture that wasn't stored is
// a no-op (appliesTo() == false). "single" and "fan" are just different ways of
// filling the map - the storage is the same either way.
//
// PresetBase is the non-template, polymorphic face: a PresetType tag + a
// virtual recall(). It lets the Engine/command layer hold and recall any preset
// without knowing its static value type. Preset<T> adds the typed storage;
// concrete types (ColorPreset, DimmerPreset) map their value onto the right
// programmer channel in recall().
//
namespace LightEngine::Engine::Pools
{

enum class PresetType
{
    DIMMER,
    COLOR,
};

inline const char *presetTypeName(PresetType t)
{
    switch (t)
    {
    case PresetType::DIMMER:
        return "Dimmer";
    case PresetType::COLOR:
        return "Color";
    }
    return "?";
}

// ---- polymorphic base -------------------------------------------------------
class PresetBase : public Engine::PoolObject
{
    PresetType m_type;

public:
    explicit PresetBase(PresetType type) : m_type(type) {}

    [[nodiscard]] PresetType type() const { return m_type; }

    // Apply this preset onto `selection` via the programmer. Only fixtures the
    // preset stored are touched; the rest are left as-is.
    virtual void recall(ProgrammerLayer &prog,
                        const DMX::FixtureGroup &selection) const = 0;
};

// ---- typed storage ----------------------------------------------------------
template <class T> class Preset : public PresetBase
{
    std::map<uint16_t, T> m_values; // FID -> stored value

public:
    using PresetBase::PresetBase;

    void set(uint16_t fid, const T &v) { m_values[fid] = v; }
    void clear() { m_values.clear(); }

    [[nodiscard]] bool appliesTo(uint16_t fid) const
    {
        return m_values.count(fid) != 0;
    }
    [[nodiscard]] const std::map<uint16_t, T> &values() const
    {
        return m_values;
    }
    [[nodiscard]] std::size_t size() const { return m_values.size(); }

    std::string describe() const override
    {
        return PoolObject::describe() + Utils::Font::colorDim + " [" +
               presetTypeName(type()) + ", " + std::to_string(m_values.size()) +
               " fixtures]" + Utils::Font::reset;
    }
};

// ---- concrete types ---------------------------------------------------------
struct HueSat
{
    float h, s;
};

struct ColorPreset : Preset<HueSat>
{
    ColorPreset() : Preset<HueSat>(PresetType::COLOR) {}

    void recall(ProgrammerLayer &prog,
                const DMX::FixtureGroup &selection) const override
    {
        for (const auto &f : selection.fixtures())
        {
            const auto it = values().find(f->Fid());
            if (it != values().end())
                prog.applyHueSat(f->Fid(), it->second.h, it->second.s);
        }
    }
};

struct DimmerPreset : Preset<float>
{
    DimmerPreset() : Preset<float>(PresetType::DIMMER) {}

    void recall(ProgrammerLayer &prog,
                const DMX::FixtureGroup &selection) const override
    {
        for (const auto &f : selection.fixtures())
        {
            const auto it = values().find(f->Fid());
            if (it != values().end())
                prog.applyIntensity(f->Fid(), it->second);
        }
    }
};

} // namespace LightEngine::Engine::Pools
