//
// Created by bobi on 31. 8. 26.
//
#include "LightEngine/Commands/Default/AstBuilder.h"

namespace LightEngine::Commands::Default
{
using Node = Parsing::Syntax::Node;

Program AstBuilder::build(const Node &entry)
{
    Program program;
    for (const Node &command : entry.kids)
        program.push_back(buildCommand(command));
    return program;
}

// command : selection at? | store | delete | clear | at
//   kids[0].rule discriminates; for "selection" an optional "at" follows.
CommandPtr AstBuilder::buildCommand(const Node &command)
{
    const Node &first = command.kids[0];
    const std::string &r = first.rule;

    if (r == "selection")
    {
        auto c = std::make_unique<SelectCmd>();
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
        auto c = std::make_unique<StoreCmd>();
        c->target = buildSelector(first.kids[1]);
        return c;
    }
    if (r == "delete") // delete : DELETE modsel
    {
        auto c = std::make_unique<DeleteCmd>();
        c->target = buildSelector(first.kids[1]);
        return c;
    }
    if (r == "clear")
        return std::make_unique<ClearCmd>();
    if (r == "at")
    {
        auto c = std::make_unique<AtCmd>();
        c->at = buildAtValue(first);
        return c;
    }
    throw std::runtime_error("buildCommand: unexpected '" + r + "'");
}

// selection : sel (op sel)*   kids = [sel, op, sel, op, sel, ...]
std::vector<Item> AstBuilder::buildItems(const Node &selection)
{
    std::vector<Item> items;

    Item first;
    first.op = ""; // leading item has no operator
    first.sel = buildSelector(selection.kids[0]);
    items.push_back(std::move(first));

    for (std::size_t i = 1; i + 1 < selection.kids.size(); i += 2)
    {
        Item item;
        item.op = selection.kids[i].kids[0].token->value; // op : PLUS|MINUS leaf
        item.sel = buildSelector(selection.kids[i + 1]);
        items.push_back(std::move(item));
    }
    return items;
}

// at : AT (atdim | atcolor | presetsel)   kids = [AT, atdim|atcolor|presetsel]
AtValue AstBuilder::buildAtValue(const Node &at)
{
    AtValue v;
    const Node &x = at.kids[1];
    if (x.rule == "presetsel")
    {
        v.kind = "preset";
        v.preset = buildPresetsel(x);
    }
    else if (x.rule == "atcolor")
    {
        // atcolor : COLOR NUM COMMA NUM COMMA NUM
        //   kids = [COLOR, NUM, COMMA, NUM, COMMA, NUM]
        v.kind = "color";
        v.r = std::stoll(x.kids[1].token->value);
        v.g = std::stoll(x.kids[3].token->value);
        v.b = std::stoll(x.kids[5].token->value);
    }
    else // atdim : NUM   kids = [NUM]
    {
        v.kind = "level";
        v.level = std::stod(x.kids[0].token->value);
    }
    return v;
}

SelectorPtr AstBuilder::buildSelector(const Node &node)
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
SelectorPtr AstBuilder::buildFixsel(const Node &fixsel)
{
    if (fixsel.kids.size() >= 3) // NUM THRU NUM
    {
        auto r = std::make_unique<FixtureRange>();
        r->from = std::stoll(fixsel.kids[0].token->value);
        r->to = std::stoll(fixsel.kids[2].token->value);
        return r;
    }
    auto f = std::make_unique<Fixture>();
    f->id = std::stoll(fixsel.kids[0].token->value);
    return f;
}

// grpsel : GROUP NUM   kids = [GROUP, NUM]
SelectorPtr AstBuilder::buildGrpsel(const Node &grpsel)
{
    auto g = std::make_unique<Group>();
    g->id = std::stoll(grpsel.kids[1].token->value);
    return g;
}

// presetsel : PRESET (DIMMER | COLOR) NUM   kids = [PRESET, kind, NUM]
SelectorPtr AstBuilder::buildPresetsel(const Node &presetsel)
{
    auto p = std::make_unique<Preset>();
    p->kind = presetsel.kids[1].token->value; // "dimmer" | "color"
    p->number = std::stoll(presetsel.kids[2].token->value);
    return p;
}
}