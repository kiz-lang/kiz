#pragma once

// 基础类型大类
enum class CTypeKind : uint8_t {
    Void,  Int,  UInt,
    LongLong,  ULongLong,
    Double,  Float, Char,
    Bool,  Ptr,  FuncPtr,
    // Struct, 
};

struct CType {
    CTypeKind kind;

    union TypeData {
        uint8_t dummy;
        uint32_t pointee_idx;
        uint32_t struct_idx;
        uint32_t func_sig_idx;
    } extra;

    bool is_const : 1;
    bool is_volatile : 1;
};

struct FFIMetaData {
    Vec<CType> params;
    Vec<Str> param_names;
    CType return_type;
    Str name;
};

struct ConstVal {

};


enum class Terminator : uint8_t {
    None,  Jmp,  JmpCond,
    Ret,  Call,  Exit
};

enum class OptLevel: uint8_t {
    Low, Middle, High
};

struct FnMetaData {
    Vec<BasicBlock> blocks;
    Str name;
    uint32_t call_count;
    OptLevel opt_level;
    bool jit_compiled;
};

struct BasicBlock {
    uint32_t start_off;
    uint32_t end_off;
    uint32_t exec_count;

    Vec<uint32_t> preds;
    Vec<uint32_t> succs;

    Terminator terminator;
    bool is_loop_head; 
    bool jit_compiled;
};

struct IRModule {
    Vec<uint8_t> ins;
    Vec<ConstVal> const_pool;
    Vec<FnMetaData> fn_pool;
    Vec<FFIMetaData> ffi_fns;

    uint16_t max_reg;

    Vec<Span> spans;
};