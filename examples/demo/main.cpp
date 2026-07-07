// Minimal demo: construct a lighting Command AST by hand (no lexer) and print
// it. Shows the typed nodes in LightEngine::Command being used directly.
#include "LightEngine/Command/Ast.h"

int main()
{
    using namespace LightEngine::Command;

    // Delete Group 1 + 4 Thru 6 - 5
    Command cmd;
    cmd.verb = Verb::Delete;
    cmd.object = ObjectType::Group;
    cmd.selection.ranges = {
        {Range::Op::Add, 1, 1},
        {Range::Op::Add, 4, 6},
        {Range::Op::Sub, 5, 5},
    };

    cmd.print();
    return 0;
}
