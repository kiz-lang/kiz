#pragma once

namespace kizc {

enum class SymKind: uint8_t {
    Const, Var
};

struct Symbol {
    Str name;
    Type symtype;
    SymKind symkind;

};

struct FunctionSymbol {
    Vec<uint32_t> param_types;
    Vec<Str> param_names;
    Str name;
    uint32_t return_type;
};

struct AbstractSymbol {
    Vec<FunctionSymbol> function_symbols;
};

struct TypeSymbol {
    
};

struct TypeVar {
    uint32_t ref = constInvalidIndex;
};

struct SymbolTable {
    Vec<Symbol> symbols;
    Vec<FunctionSymbol> function_symbols;
    Vec<AbstractSymbol> abstract_symbols;
    Vec<TypeSymbol> type_symbols;
    Vec<TypeVar> type_vars;
    bool is_loop: 1;
    bool loop_head_block: 1;
    bool loop_end_block: 1;
};

}