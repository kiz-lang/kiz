#pragma once


class Vm : NoCopyMove {
    Stack stack_;
    Object gc_root_;
    GarbageCollection& gc_;
    JitCompiler& jitc_;
    Ptr<IRModule> irmodule_;
    Vec<Value> static_mem_;

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