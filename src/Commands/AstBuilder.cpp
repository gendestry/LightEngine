#include "LightEngine/Commands/ast/AstBuilder.h"

#include <stdexcept>
#include <string>

namespace LightEngine::Commands
{
using Node = Parsing::Syntax::Node;

// entry : command+     kids = [command, ...]
Macros::Program AstBuilder::build(const Node &entry)
{
    Macros::Program program;
    for (const Node &command : entry.kids)
        program.push_back(buildCommand(command));
    return program;
}

// command : selection at? | store | delete | clear | at
//   kids[0].rule discriminates; for "selection" an optional "at" follows.
Macros::CommandPtr AstBuilder::buildCommand(const Node &command)
{
    const Node &first = command.kids[0];
    const std::string &r = first.rule;

    if (r == "selection")
    {
        auto c = std::make_unique<Macros::SelectCmd>();
        c->items = buildItems(first);
        if (command.kids.size() > 1 && command.kids[1].rule == "at")
        {
            c->hasAt = true;
            c->at = buildAtValue(command.kids[1]);
        }
        return c;
    }
    if (r == "store") // store : STORE modsel   kids = [STORE, modsel]
    {
        auto c = std::make_unique<Macros::StoreCmd>();
        c->target = buildSelector(first.kids[1]);
        return c;
    }
    if (r == "delete") // delete : DELETE modsel
    {
        auto c = std::make_unique<Macros::DeleteCmd>();
        c->target = buildSelector(first.kids[1]);
        return c;
    }
    if (r == "clear")
        return std::make_unique<Macros::ClearCmd>();
    if (r == "at")
    {
        auto c = std::make_unique<Macros::AtCmd>();
        c->at = buildAtValue(first);
        return c;
    }
    throw std::runtime_error("buildCommand: unexpected '" + r + "'");
}

// selection : sel (op sel)*   kids = [sel, op, sel, op, sel, ...]
std::vector<Macros::Item> AstBuilder::buildItems(const Node &selection)
{
    std::vector<Macros::Item> items;

    Macros::Item first;
    first.op = ""; // leading item has no operator
    first.sel = buildSelector(selection.kids[0]);
    items.push_back(std::move(first));

    for (std::size_t i = 1; i + 1 < selection.kids.size(); i += 2)
    {
        Macros::Item item;
        item.op = selection.kids[i].kids[0].token->value; // op : PLUS|MINUS leaf
        item.sel = buildSelector(selection.kids[i + 1]);
        items.push_back(std::move(item));
    }
    return items;
}

// at : AT (NUM | presetsel)   kids = [AT, NUM] | [AT, presetsel]
Macros::AtValue AstBuilder::buildAtValue(const Node &at)
{
    Macros::AtValue v;
    const Node &x = at.kids[1];
    if (x.rule == "presetsel")
    {
        v.isPreset = true;
        v.preset = buildPresetsel(x);
    }
    else
    {
        v.isPreset = false;
        v.level = std::stod(x.token->value);
    }
    return v;
}

Macros::SelectorPtr AstBuilder::buildSelector(const Node &node)
{
    // sel : fixsel | grpsel   /   modsel : grpsel | presetsel  -> unwrap
    const Node *p = &node;
    if (p->rule == "sel" || p->rule == "modsel")
        p = &p->kids[0];

    if (p->rule == "fixsel")
        return buildFixsel(*p);
    if (p->rule == "grpsel")
        return buildGrpsel(*p);
    if (p->rule == "presetsel")
        return buildPresetsel(*p);
    throw std::runtime_error("buildSelector: unexpected '" + p->rule + "'");
}

// fixsel : NUM (THRU NUM)?   kids = [NUM] | [NUM, THRU, NUM]
Macros::SelectorPtr AstBuilder::buildFixsel(const Node &fixsel)
{
    if (fixsel.kids.size() >= 3) // NUM THRU NUM
    {
        auto r = std::make_unique<Macros::FixtureRange>();
        r->from = std::stoll(fixsel.kids[0].token->value);
        r->to = std::stoll(fixsel.kids[2].token->value);
        return r;
    }
    auto f = std::make_unique<Macros::Fixture>();
    f->id = std::stoll(fixsel.kids[0].token->value);
    return f;
}

// grpsel : GROUP NUM   kids = [GROUP, NUM]
Macros::SelectorPtr AstBuilder::buildGrpsel(const Node &grpsel)
{
    auto g = std::make_unique<Macros::Group>();
    g->id = std::stoll(grpsel.kids[1].token->value);
    return g;
}

// presetsel : PRESET NUM DOT NUM   kids = [PRESET, NUM, DOT, NUM]
Macros::SelectorPtr AstBuilder::buildPresetsel(const Node &presetsel)
{
    auto p = std::make_unique<Macros::Preset>();
    p->bank = std::stoll(presetsel.kids[1].token->value);
    p->number = std::stoll(presetsel.kids[3].token->value);
    return p;
}

} // namespace LightEngine::Commands
