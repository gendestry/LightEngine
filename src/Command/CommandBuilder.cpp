// =============================================================================
//  CommandBuilder: recursive-descent lowering of a token line into a Command.
// =============================================================================
//
//      command   : verb object selection (AT level)?
//      verb      : STORE | DELETE
//      object    : FIXTURE | GROUP | COLOR | CUE | PRESET
//      selection : range ((PLUS|MINUS) range)*
//      range     : NUM (THRU NUM)?
//      level     : FULL | NUM

#include "LightEngine/Command/CommandBuilder.h"

#include <stdexcept>

namespace LightEngine::Command
{
    Command CommandBuilder::build(const std::vector<Token> &tokens)
    {
        toks = &tokens;
        pos = 0;

        Command cmd;
        cmd.verb = parseVerb();
        cmd.object = parseObject();
        cmd.selection = parseSelection();
        if (match("AT"))
            cmd.level = parseLevel();

        if (!atEnd())
            throw std::runtime_error("trailing tokens after command near '" + peek().value + "'");
        return cmd;
    }

    // ---- cursor helpers -----------------------------------------------------
    bool CommandBuilder::atEnd() const { return pos >= toks->size(); }

    const CommandBuilder::Token &CommandBuilder::peek() const
    {
        if (atEnd())
            throw std::runtime_error("unexpected end of command");
        return (*toks)[pos];
    }

    const CommandBuilder::Token &CommandBuilder::advance() { return (*toks)[pos++]; }

    bool CommandBuilder::check(const std::string &name) const
    {
        return !atEnd() && (*toks)[pos].name == name;
    }

    bool CommandBuilder::match(const std::string &name)
    {
        if (!check(name))
            return false;
        ++pos;
        return true;
    }

    const CommandBuilder::Token &CommandBuilder::expect(const std::string &name)
    {
        if (!check(name))
        {
            const std::string got = atEnd() ? "end of input" : ("'" + peek().value + "'");
            throw std::runtime_error("expected " + name + " but got " + got);
        }
        return advance();
    }

    // ---- rules --------------------------------------------------------------
    Verb CommandBuilder::parseVerb()
    {
        if (match("STORE"))
            return Verb::Store;
        if (match("DELETE"))
            return Verb::Delete;
        throw std::runtime_error("expected a verb (Store|Delete) near '" + peek().value + "'");
    }

    ObjectType CommandBuilder::parseObject()
    {
        if (match("FIXTURE"))
            return ObjectType::Fixture;
        if (match("GROUP"))
            return ObjectType::Group;
        if (match("COLOR"))
            return ObjectType::Color;
        if (match("CUE"))
            return ObjectType::Cue;
        if (match("PRESET"))
            return ObjectType::Preset;
        throw std::runtime_error("expected an object type near '" + peek().value + "'");
    }

    Selection CommandBuilder::parseSelection()
    {
        Selection sel;
        sel.ranges.push_back(parseRange(Range::Op::Add)); // first range: implicit +
        while (true)
        {
            if (match("PLUS"))
                sel.ranges.push_back(parseRange(Range::Op::Add));
            else if (match("MINUS"))
                sel.ranges.push_back(parseRange(Range::Op::Sub));
            else
                break;
        }
        return sel;
    }

    Range CommandBuilder::parseRange(Range::Op op)
    {
        Range r;
        r.op = op;
        r.from = std::stoll(expect("NUM").value);
        r.to = r.from;
        if (match("THRU"))
            r.to = std::stoll(expect("NUM").value);
        return r;
    }

    Level CommandBuilder::parseLevel()
    {
        Level lvl;
        if (match("FULL"))
            lvl.full = true;
        else
            lvl.value = std::stoll(expect("NUM").value);
        return lvl;
    }
}
