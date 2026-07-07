#pragma once
// =============================================================================
//  Token-stream -> typed Command AST.
// =============================================================================
//
//  grammar/demo/lang.syn currently has no rules, so instead of running the
//  grammar Engine we lower the lexed token stream directly into a Command.
//  When a real grammar exists this can be swapped for a CST walker (see the
//  AstBuilder pattern in lib/NewSyntaxParser/src/test).

#include "LightEngine/Command/Ast.h"
#include "Tokenizer/Parser.h" // Parsing::Tokenizer::Token (Token.h is empty)

#include <vector>

namespace LightEngine::Command
{
    class CommandBuilder
    {
    public:
        // Build a Command from the (non-ignored) tokens of a single line.
        // Throws std::runtime_error on anything that does not match the grammar.
        Command build(const std::vector<Parsing::Tokenizer::Token> &tokens);

    private:
        using Token = Parsing::Tokenizer::Token;

        const std::vector<Token> *toks = nullptr;
        std::size_t pos = 0;

        bool atEnd() const;
        const Token &peek() const;
        const Token &advance();
        bool check(const std::string &name) const;
        bool match(const std::string &name);
        const Token &expect(const std::string &name);

        Verb parseVerb();
        ObjectType parseObject();
        Selection parseSelection();
        Range parseRange(Range::Op op);
        Level parseLevel();
    };
}
