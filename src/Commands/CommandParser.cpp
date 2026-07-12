#include "LightEngine/Commands/CommandParser.h"

#include "LightEngine/Commands/ast/AstBuilder.h"

#include "Syntax/Engine.h"
#include "Syntax/GrammarParser.h"
#include "Tokenizer/Parser.h"

namespace LightEngine::Commands
{
CommandParser::CommandParser(const std::string &tokensFile,
                             const std::string &grammarFile)
    : m_grammar(Parsing::Syntax::GrammarParser::parseFile(grammarFile)),
      m_tokensFile(tokensFile)
{
}

Macros::Program CommandParser::parse(const std::string &line)
{
    // 1. lex the line, dropping ignored tokens (whitespace)
    Parsing::Tokenizer::Parser lexer(m_tokensFile);
    lexer.parseString(line);

    std::vector<Parsing::Tokenizer::Token> tokens;
    for (auto &t : lexer.getTokens())
        if (!t.ignore)
            tokens.push_back(t);

    // 2. parse into a CST
    Parsing::Syntax::Engine engine(m_grammar, tokens);
    auto cst = engine.parse(m_grammar.startRule);
    if (!cst)
        return {};

    // 3. lower CST -> typed AST
    AstBuilder builder;
    return builder.build(*cst);
}
} // namespace LightEngine::Commands
