#pragma once
#include <vector>

#include "LightEngine/Commands/Default/Ast.h"
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
//   AtCmd      ->  set intensity/color, or recall a preset
//
namespace LightEngine::Commands::Default
{
class CommandExecutor : public CommandVisitor
{
    Engine::Engine &m_engine;

public:
    explicit CommandExecutor(Engine::Engine &engine) : m_engine(engine) {}

    // Run a whole program (sequence of commands), in order.
    void run(Program &program);

    // ---- CommandVisitor ----
    void visit(SelectCmd &c) override;
    void visit(StoreCmd &c) override;
    void visit(DeleteCmd &c) override;
    void visit(ClearCmd &c) override;
    void visit(AtCmd &c) override;

private:
    // Expand one selector into the fixtures it names (Preset -> empty).
    std::vector<uint16_t> expand(Selector &sel) const;

    // Resolve a SelectCmd's item list, applying '+'/'-' in order, into the
    // final ordered fid list handed to the programmer.
    std::vector<uint16_t> resolveSelection(SelectCmd &c) const;

    // Apply an 'at': a bare number sets intensity (level/100), a color sets
    // hue/sat from r,g,b, a preset ref recalls that preset onto the selection.
    void applyAt(const AtValue &at);
};
}