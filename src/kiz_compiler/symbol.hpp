#pragma once

namespace kizc {

enum class SymKind: uint8_t {
    Const, Var, Function
};

struct Symbol {
    Str name;
    Type symtype;
    SymKind symkind;
};

struct SymbolTable {
    Vec<Symbol> symbols;
};


}