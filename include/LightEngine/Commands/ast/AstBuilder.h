#pragma once
#include <vector>

#include "LightEngine/Commands/ast/commands_ast.h"
#include "Syntax/Node.h"

//
// AstBuilder: lowers the parser's homogeneous CST (Parsing::Syntax::Node) into
// the typed command AST (Macros::). One build* per grammar rule; children are
// read by position, per the Engine's flattening:
//   Terminal/Literal -> leaf;  RuleRef -> child node;  Seq/Star/Optional/( )
//   flatten into the parent;  Choice -> only the winning alternative.
//
namespace LightEngine::Commands
{
class AstBuilder
{
    using Node = Parsing::Syntax::Node;

public:
    // entry : command+
    Macros::Program build(const Node &entry);

private:
    Macros::CommandPtr buildCommand(const Node &command);
    std::vector<Macros::Item> buildItems(const Node &selection);
    Macros::AtValue buildAtValue(const Node &at);

    // sel / modsel wrappers unwrap to fixsel | grpsel | presetsel.
    Macros::SelectorPtr buildSelector(const Node &node);
    Macros::SelectorPtr buildFixsel(const Node &fixsel);
    Macros::SelectorPtr buildGrpsel(const Node &grpsel);
    Macros::SelectorPtr buildPresetsel(const Node &presetsel);
};
} // namespace LightEngine::Commands
