#pragma once

namespace kizc {

enum class SymKind: uint8_t {
    Const, Var, Function
};

struct Symbol {
    Str name;
    SymKind symkind;
};

struct StructItemInfo {
    Str name;
    uint32_t typeid;
};

enum class TypeKind: uint8_t {
    Int, Decimal, Array,
    Struct, SumType, Alias,
};

struct Type {
    uint32_t typeid;
    TypeKind typekind;
    Str typename;
    union {
        uint32_t case_array_count;
        Vec<StructItemInfo> case_struct_info;
        Vec<uint32_t> case_sumtypes;
        uint32_t case_alias_ref;
    } typeinfo;
};

struct SymbolTable {
    Vec<Symbol> symbols;
};


}