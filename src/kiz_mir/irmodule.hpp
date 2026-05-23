#pragma once

enum class CTypeKind : uint8_t {
    Void,  Int,  UInt,
    LongLong,  ULongLong,
    Double,  Float, Char,
    Bool,  Ptr,  FuncPtr,
};

struct CType {
    union {
        uint32_t case_ptr_ref;
    } data;

    CTypeKind kind;

    bool is_const;
    bool is_volatile;
};

struct FFIMetaData {
    Vec<uint32_t> params;
    Vec<Str> param_names;
    uint32_t return_type;
    Str name;
};

enum class ConstType : uint8_t {
    Int, Decimal, String
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
    Goto,  GotoIfFalse, GotoIfTrue,
    Ret,  Call,
};

enum class OptLevel : uint8_t {
    Low, Middle, High
};

struct FnMetaData {
    Vec<uint32_t> blocks;
    Str name;
    uint32_t call_count;
    OptLevel opt_level;
    bool jit_compiled;
};

struct BasicBlock {
    uint32_t start_offset;
    uint32_t end_offset;
    uint32_t exec_count;

    Vec<uint32_t> preds;
    Vec<uint32_t> succs;

    Terminator terminator;
    uint8_t terminator_op_number;
    bool is_loop_head; 
    bool jit_compiled;
};

struct IRModule {
    Vec<uint8_t> ins;
    Vec<ConstVal> const_pool;
    Vec<FnMetaData> fn_pool;
    Vec<FFIMetaData> ffi_fns;
    Vec<CType> ffi_types;
    Vec<BasicBlock> basic_blocks;

    Vec<Span> spans;

    uint32_t entry_fn;

    uint16_t max_reg;
};