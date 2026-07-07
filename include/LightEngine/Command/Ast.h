#pragma once
// =============================================================================
//  Typed Command AST for the lighting-console command language.
// =============================================================================
//
//  Grammar (see grammar/demo/lang.tok), by example:
//
//      Store Group 1 Thru 5 @ Full
//      Delete Fixture 1 + 2 + 3
//      Store Preset 4
//
//      command   : verb object selection ( AT level )?
//      verb      : STORE | DELETE
//      object    : FIXTURE | GROUP | COLOR | CUE | PRESET
//      selection : range ( (PLUS|MINUS) range )*
//      range     : NUM ( THRU NUM )?
//      level     : FULL | NUM
//
//  Data-only nodes with a small print() so a parsed command can be inspected
//  without executing it. Counterpart of lib/NewSyntaxParser test/Ast.h, but
//  specialised for this project's command grammar.

#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace LightEngine::Command
{
    inline std::string pad(int n) { return std::string(n, ' '); }

    // ---- verb : STORE | DELETE ----------------------------------------------
    enum class Verb
    {
        Store,
        Delete
    };
    inline const char *toString(Verb v)
    {
        switch (v)
        {
        case Verb::Store:
            return "Store";
        case Verb::Delete:
            return "Delete";
        }
        return "?";
    }

    // ---- object : FIXTURE | GROUP | COLOR | CUE | PRESET ---------------------
    enum class ObjectType
    {
        Fixture,
        Group,
        Color,
        Cue,
        Preset
    };
    inline const char *toString(ObjectType o)
    {
        switch (o)
        {
        case ObjectType::Fixture:
            return "Fixture";
        case ObjectType::Group:
            return "Group";
        case ObjectType::Color:
            return "Color";
        case ObjectType::Cue:
            return "Cue";
        case ObjectType::Preset:
            return "Preset";
        }
        return "?";
    }

    // ---- range : NUM (THRU NUM)? --------------------------------------------
    // A single id (from == to) or an inclusive span "from Thru to". The `op`
    // says how this range combines with the running selection: the first range
    // is always Add, later ones carry the + / - that preceded them.
    struct Range
    {
        enum class Op
        {
            Add, // '+' (also the implicit op of the first range)
            Sub  // '-'
        } op = Op::Add;
        long long from = 0;
        long long to = 0;

        bool isSpan() const { return to != from; }

        std::string toString() const
        {
            std::string s = op == Op::Sub ? "- " : "+ ";
            s += std::to_string(from);
            if (isSpan())
                s += " Thru " + std::to_string(to);
            return s;
        }
    };

    // ---- selection : range ((PLUS|MINUS) range)* ----------------------------
    struct Selection
    {
        std::vector<Range> ranges;
    };

    // ---- level : FULL | NUM -------------------------------------------------
    struct Level
    {
        bool full = false; // true for FULL; otherwise `value` holds the NUM
        long long value = 0;

        std::string toString() const
        {
            return full ? "Full" : std::to_string(value);
        }
    };

    // ---- command : verb object selection (AT level)? ------------------------
    struct Command
    {
        Verb verb = Verb::Store;
        ObjectType object = ObjectType::Fixture;
        Selection selection;
        std::optional<Level> level; // present only when "@ level" was given

        void print(int indent = 0) const
        {
            std::cout << pad(indent) << "Command " << LightEngine::Command::toString(verb)
                      << " " << LightEngine::Command::toString(object) << "\n";
            std::cout << pad(indent + 2) << "Selection\n";
            for (const auto &r : selection.ranges)
                std::cout << pad(indent + 4) << r.toString() << "\n";
            if (level)
                std::cout << pad(indent + 2) << "@ " << level->toString() << "\n";
        }
    };
}
