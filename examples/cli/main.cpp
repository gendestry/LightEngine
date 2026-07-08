// CLI demo: lex a mixer command with the NewSyntaxParser tokenizer, lower the
// token stream into the typed LightEngine::Command AST and print it.
//
// grammar/demo/lang.syn currently has no rules, so instead of running the
// grammar Engine the CommandBuilder walks the lexed tokens directly.
#include "LightEngine/Command/CommandBuilder.h"
#include "Tokenizer/Parser.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef MIXER_GRAMMAR_DIR
#define MIXER_GRAMMAR_DIR "grammar"
#endif

int main(int argc, char **argv)
{
    const std::string grammarDir = MIXER_GRAMMAR_DIR;
    const std::string tokenFile = grammarDir + "/demo/lang.tok";
    const std::string inputFile =
        argc > 1 ? argv[1] : grammarDir + "/demo/input.txt";

    // --- 1. Lex the input into tokens ------------------------------------
    Parsing::Tokenizer::Parser lexer(tokenFile);
    if (!lexer.parse(inputFile))
    {
        std::cerr << "lexing failed for '" << inputFile << "'\n";
        return 1;
    }

    // --- 2. Keep only the meaningful (non-ignored) tokens ----------------
    std::vector<Parsing::Tokenizer::Token> tokens;
    for (const auto &t : lexer.getTokens())
        if (!t.ignore)
            tokens.push_back(t);

    // --- 3. Lower the tokens into the typed Command AST and print it ------
    try
    {
        LightEngine::Command::CommandBuilder builder;
        LightEngine::Command::Command cmd = builder.build(tokens);

        std::cout << "=== Command AST for " << inputFile << " ===\n";
        cmd.print();
    }
    catch (const std::exception &e)
    {
        std::cerr << "parse error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
