#pragma once

struct ExternalFn {

};

struct ConstVal {

};


enum class Terminator : uint8_t {
    None,  Jmp,  JmpCond,
    Ret,  Call,  Exit
};

struct FnMetaData {
    Vec<BasicBlock> blocks;
    uint32_t call_count;
    bool jit_compiled;
};

struct BasicBlock {
    uint32_t start_off;
    uint32_t end_off;
    uint32_t exec_count;

    uint32_t pred;
    uint32_t succ;

    Terminator terminator;
    bool is_loop_head; 
    bool jit_compiled;
};

struct IRModule {
    Vec<uint8_t> ins;
    Vec<ConstVal> const_pool;
    Vec<FnMetaData> fn_pool;
    Vec<ExternalFn> external_fns;

    uint16_t max_reg;

    Vec<Span> line_map;
};