#pragma once
#include "commands_ast.h"

namespace Macros
{
struct ASTSelect : public SelectorVisitor
{
    void visit(Fixture &f) {}
    void visit(FixtureRange &fr) {}
    void visit(Group &grp) {}
    void visit(Preset &p) {}
};
} // namespace Macros