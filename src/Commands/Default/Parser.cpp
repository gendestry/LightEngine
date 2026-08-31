#include "LightEngine/Commands/Default/Parser.h"
#include "LightEngine/Commands/Default/AstBuilder.h"

#include "Syntax/Engine.h"
#include "Syntax/GrammarParser.h"
#include "SyntaxParser/Tokenizer/Parser.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace LightEngine::Commands::Default
{
namespace
{
Parsing::Syntax::Grammar loadGrammar(const std::string &grammarFile)
{
    auto grammar = Parsing::Syntax::GrammarParser::parseFile(grammarFile);
    if (!grammar)
        throw std::runtime_error("CommandParser: failed to load grammar file: " + grammarFile);
    return std::move(*grammar);
}
}

CommandParser::CommandParser(const std::string &tokensFile,
                             const std::string &grammarFile)
                                 :  m_tokensFile(tokensFile),
                                    m_lexer(tokensFile),
                                    m_grammar(loadGrammar(grammarFile))
{
}

void CommandParser::buildProgram(const Parsing::Syntax::Node &cst)
{
    AstBuilder builder;
    m_program = builder.build(cst);
}

Program CommandParser::parse(const std::string &line)
{
    // 1. lex the line, dropping ignored tokens (whitespace)
    auto& lexer = m_lexer;
    lexer.parseString(line);

    std::vector<Parsing::Tokenizer::Token> tokens;
    for (auto &t : lexer.getTokens())
        if (!t.ignore)
            tokens.push_back(t);

    // 2. parse into a CST
    Parsing::Syntax::Engine engine(m_grammar, tokens);
    auto cst = engine.parse(m_grammar.startRule);
    if (!cst)
    {
        m_program.clear();
        return std::move(m_program);
    }

    // 3. lower CST -> typed AST, storing it on m_program
    buildProgram(*cst);
    return std::move(m_program);
}
} // namespace LightEngine::Com