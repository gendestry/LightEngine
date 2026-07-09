#pragma once
#include "Syntax/Node.h" // Parsing::Syntax::Node

#include "commands_ast.h"

namespace Macros
{
using Node = Parsing::Syntax::Node;
struct Builder
{
    Program build(const Node &entry) {}
};

} // namespace Macros