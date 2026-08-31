#pragma once
#include <string>

#include "LightEngine/Commands/Default/Ast.h"
#include "Syntax/Grammar.h"
#include "Syntax/Node.h"
#include "SyntaxParser/Tokenizer/Parser.h"

//
// CommandParser: text -> tokens -> CST -> typed AST. Loads the grammar once;
// each parse() lexes a line (via the token defs), parses it into a CST, and
// lowers that CST into m_program. Returns an empty Program on a lex/parse
// error.
//
namespace LightEngine::Commands::Default
{
class CommandParser
{
    Parsing::Tokenizer::Parser m_lexer;
    Parsing::Syntax::Grammar m_grammar;
    std::string m_tokensFile;
    Program m_program;

    // Lowers a CST into the typed AST, storing the result in m_program.
    void buildProgram(const Parsing::Syntax::Node &cst);

public:
    CommandParser(const std::string &tokensFile, const std::string &grammarFile);

    [[nodiscard]] Program parse(const std::string &line);
    [[nodiscard]] const Program &program() const { return m_program; }
};
} //