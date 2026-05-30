#pragma once

enum class CTypeKind : uint8_t {
    Void,  
    Int,  
    UInt,
    LongLong,
    ULongLong,
    Double,
    Float,
    Char,
    Bool,
    Ptr,
    FuncPtr,
};

struct CType {
    union {
        uint32_t case_ptr_ref;
    } data;

    CTypeKind kind;

    bool is_const;
    bool is_volatile;
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
};

struct Type {
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


struct FFIMetaData {
    Vec<uint32_t> params;
    Vec<Str> param_names;
    uint32_t return_type;
    Str name;
};

enum class ConstType : uint8_t {
    Int,
    Decimal,
    String
};

struct ConstVal {
    union {
        uint64_t case_int;
        double case_decimal;
        Str case_str;
    } data;
    ConstType kind;
};


enum class Terminator : uint8_t {
    Goto,  
    GotoIfFalse,
    GotoIfTrue,
    Cmp,
    Ret,
    Call,
};

struct Effect {
    Str effect_name;
    uint32_t effect_id;
    uint32_t argv_type; // Tuple
    uint32_t default_impl;
};

struct Handler {
    static constexpr size_t constMaxBind = 32;

    uint32_t effect_id[constMaxBind];
    uint32_t fn_idx[constMaxBind];
};

enum class OptLevel : uint8_t {
    Low,
    Middle,
    High
};

struct FnMetaData {
    Vec<uint32_t> blocks;
    Str name;
    uint32_t call_count;
    OptLevel opt_level;
    bool jit_compiled;
};

struct BasicBlock {
    Vec<uint32_t> preds;
    Vec<uint32_t> succs;
    uint32_t start_offset;
    uint32_t end_offset;
    uint32_t exec_count;

    uint32_t terminator_opnum;
    Terminator terminator;
    bool is_loop_head; 
    bool jit_compiled;
};

struct IRModule {
    Vec<uint8_t> ins;
    Vec<ConstVal> const_pool;
    Vec<FnMetaData> fn_pool;
    Vec<Type> type_pool;
    Vec<Effect> effect_pool;
    Vec<Handler> handler_pool;
    Vec<FFIMetaData> ffi_fns;

    Vec<BasicBlock> basic_blocks;

    Vec<Span> spans;

    uint32_t entry_fn;

    uint16_t max_reg;
};