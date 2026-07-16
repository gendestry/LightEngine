# =============================================================================
#  commands.spec — AST for the fixture/group command grammar (commands.syn).
#  Consumed by gen_ast.py -> commands_ast.h.
#
#    entry     : command+                           -> a sequence of Command
#    command   : selection at? | store | delete
#              | clear | at
#    selection : sel (op sel)*                      -> SelectCmd (list of Item)
#    sel       : fixsel | grpsel                    -> Fixture|FixtureRange|Group
#    modsel    : grpsel | presetsel                 -> Group|Preset
#    fixsel    : NUM (THRU NUM)?
#    grpsel    : GROUP NUM
#    presetsel : PRESET (DIMMER | COLOR) NUM
#    store     : STORE modsel
#    delete    : DELETE modsel
#    clear     : CLEAR
#    atdim     : NUM
#    atcolor   : COLOR NUM COMMA NUM COMMA NUM
#    at        : AT (atdim | atcolor | presetsel)
# =============================================================================

namespace Macros

# A program is a sequence of commands.
program Command[]

# ---- category: the selectable / addressable objects ----------------------
category Selector {
    Fixture      { i64 id }                # fixsel single    e.g. 13
    FixtureRange { i64 from; i64 to }       # fixsel range     e.g. 1 thru 10
    Group        { i64 id }                # grpsel           e.g. group 4
    Preset       { string kind; i64 number } # presetsel      e.g. preset color 5
}

# ---- record: the value an 'at' applies -----------------------------------
#   kind determines which field carries the payload:
#     "level"  -> at NUM               level holds the number      (atdim)
#     "color"  -> at color r,g,b       r/g/b hold the components   (atcolor)
#     "preset" -> at preset (d|c) N    preset holds a Preset       (presetsel)
record AtValue {
    string   kind
    f64      level
    i64      r
    i64      g
    i64      b
    Selector preset
}

# ---- category: the top-level commands ------------------------------------
category Command {
    SelectCmd { Item[] items; bool hasAt; AtValue at }  # selection at?
    StoreCmd  { Selector target }                       # store modsel
    DeleteCmd { Selector target }                       # delete modsel
    ClearCmd  { }                                       # clear
    AtCmd     { AtValue at }                             # bare at
}

# ---- record: pairs each selector with the operator that precedes it ------
#   op is "+" / "-"  (empty for the first item, which has no leading op)
record Item {
    string   op
    Selector sel
}
