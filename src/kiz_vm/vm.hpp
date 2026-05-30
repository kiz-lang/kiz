#pragma once


struct Segment {
    Value* mem_start;
    Value* mem_end;
    uint32_t bind_handler;
    uint32_t pred;
    uint32_t succ;
    uint32_t pc;
};


class Vm {
    /*
    Segment stack_;
    Segment tmp_continuation_;
    */
    Ptr<IRModule> irmodule_;
    JitCompiler& jitc_;
    Vec<Value> static_mem_;
    Vec<Value> stack_mem_;

    /*
    MemoryPool<Value> stack_mem_pool;
    Vec<Segment> segment_pool;
    uint32_t current_segement_;
    uint32_t effect_id;
    */
    uint32_t pc_ = 0;

    auto exec_unit() noexcept -> void;

    auto new_frame() noexcept -> void;
    auto destory_frame() noexcept -> void;

    auto jit_compile() noexcept -> RuntimeError;
    auto run_jit_code() noexcept -> void;
    auto refresh_jit() -> void;
    auto is_jit_ready() const noexcept -> bool;


public:
    explicit Vm(Ptr<IRModule> irmod, JitCompiler& jitc)
    : irmodule_(irmod), jitc_(jitc) {}

    auto run() noexcept -> void;
};