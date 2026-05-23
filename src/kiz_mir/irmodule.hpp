#pragma once

struct ExternalFn {

};

struct ConstVal {

};


enum class Terminator : uint8_t {
    None,  Jmp,  JmpCond,
    Ret,  Call,  Exit
};

struct BasicBlock {
    uint32_t start_off;
    uint32_t end_off;

    Terminator terminator;

    Vec<uint32_t> pred;
    Vec<uint32_t> succ;
};

struct IRModule {
    Vec<uint8_t> ins;
    Vec<BasicBlock> blocks;
    Vec<ConstVal> const_pool;
    Vec<ExternalFn> external_fns;

    uint16_t max_reg;

    Vec<Span> line_map;
};