#pragma once
#include <string>

#include "LightEngine/Commands/ast/commands_ast.h"
#include "Syntax/Grammar.h"

//
// CommandParser: text -> tokens -> CST -> typed AST. Loads the grammar once;
// each parse() lexes a line (via the token defs) and builds a Program. Returns
// an empty Program on a lex/parse error.
//
namespace LightEngine::Commands
{
class CommandParser
{
    Parsing::Syntax::Grammar m_grammar;
    std::string m_tokensFile;

public:
    CommandParser(const std::string &tokensFile, const std::string &grammarFile);

    [[nodiscard]] Macros::Program parse(const std::string &line);
};
} // namespace LightEngine::Commands
