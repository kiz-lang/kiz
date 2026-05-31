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
    Vec<uint32_t> impl_abstracts;
    Vec<FunctionSymbol> function_symbols;
};

struct StructItemInfo {
    Str name;
    uint32_t tyid;
};

enum class TypeKind: uint8_t {
    Int,
    Decimal,
    Array,
    Struct,
    SumType,
    Alias,
    Tuple,
    CType,
    Opaque,
};

struct Type {
    Vec<uint32_t> impl_abstracts;
    union {
        uint32_t case_array_count;
        uint32_t case_alias_ref;
        Vec<StructItemInfo> case_struct_info;
        Vec<uint32_t> case_sumtypes;
        Vec<uint32_t case_tuple;
        CType case_ctype;
    } typeinfo;

    Str tyname;

    TypeKind typekind;
    uint32_t tyid;
};

struct TypeVar {
    uint32_t ref = constInvalidIndex;
};

struct SymbolTable {
    Vec<Symbol> symbols;

    Vec<TypeVar> type_vars;
    bool is_loop: 1;
    bool loop_head_block: 1;
    bool loop_end_block: 1;
};

}