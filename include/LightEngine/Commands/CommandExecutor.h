#pragma once
#include <cstdint>
#include <vector>

#include "LightEngine/Commands/ast/commands_ast.h"
#include "LightEngine/Engine/Engine.h"

//
// CommandExecutor: walks a parsed command AST (Macros::Program) and drives the
// Engine facade. This is the AST -> action half of the command pipeline; a
// separate AstBuilder produces the Program from parsed text.
//
//   SelectCmd  ->  select fixtures/groups (+/-), then optional 'at'
//   StoreCmd   ->  storeGroup / storeColorPreset|storeDimmerPreset
//   DeleteCmd  ->  pool.remove
//   ClearCmd   ->  engine.clear()
//   AtCmd      ->  set intensity, or recall a preset
//
namespace LightEngine::Commands
{
class CommandExecutor : public Macros::CommandVisitor
{
    Engine::Engine &m_engine;

public:
    explicit CommandExecutor(Engine::Engine &engine) : m_engine(engine) {}

    // Run a whole program (sequence of commands), in order.
    void run(Macros::Program &program);

    // ---- CommandVisitor ----
    void visit(Macros::SelectCmd &c) override;
    void visit(Macros::StoreCmd &c) override;
    void visit(Macros::DeleteCmd &c) override;
    void visit(Macros::ClearCmd &c) override;
    void visit(Macros::AtCmd &c) override;

private:
    // Expand one selector into the fixtures it names (Preset -> empty).
    std::vector<uint16_t> expand(Macros::Selector &sel) const;

    // Resolve a SelectCmd's item list, applying '+'/'-' in order, into the
    // final ordered fid list handed to the programmer.
    std::vector<uint16_t> resolveSelection(Macros::SelectCmd &c) const;

    // Apply an 'at': a bare number sets intensity (level/100), a preset ref
    // recalls that preset onto the current selection.
    void applyAt(const Macros::AtValue &at);

    // Preset addressing: bank picks the pool, number is the slot.
    //   bank 1 -> dimmer, bank 2 -> color
    enum class PresetKind
    {
        Dimmer,
        Color,
        Unknown,
    };
    static PresetKind presetKind(long long bank);
};
} // namespace LightEngine::Commands
