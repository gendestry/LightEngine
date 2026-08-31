#pragma once
#include <vector>

#include "LightEngine/Commands/Default/Ast.h"
#include "Syntax/Node.h"

//
// AstBuilder: lowers the parser's homogeneous CST (Parsing::Syntax::Node) into
// the typed command AST (Macros::). One build* per grammar rule; children are
// read by position, per the Engine's flattening:
//   Terminal/Literal -> leaf;  RuleRef -> child node;  Seq/Star/Optional/( )
//   flatten into the parent;  Choice -> only the winning alternative.
//
namespace LightEngine::Commands::Default
{
class AstBuilder
{
    using Node = Parsing::Syntax::Node;

public:
    // entry : command+
    Program build(const Node &entry);

private:
    CommandPtr buildCommand(const Node &command);
    std::vector<Item> buildItems(const Node &selection);
    AtValue buildAtValue(const Node &at);

    // sel / modsel wrappers unwrap to fixsel | grpsel | presetsel.
    SelectorPtr buildSelector(const Node &node);
    SelectorPtr buildFixsel(const Node &fixsel);
    SelectorPtr buildGrpsel(const Node &grpsel);
    SelectorPtr buildPresetsel(const Node &presetsel);
};
}