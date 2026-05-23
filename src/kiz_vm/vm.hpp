#pragma once

class Vm {
    Ptr<IRModule> irmodule_;
    JitCompiler& jitc_;
    Vec<Value> stack_;
    uint32_t pc_ = 0;

    auto exec_unit() noexcept -> void;

    auto new_frame() noexcept -> void;
    auto destory_frame() noexcept -> void;

    auto jit_compile() -> bool;
    auto run_jit_code() -> Value;
    auto refresh_jit() -> void;
    auto is_jit_ready() const noexcept -> bool;


    [[noreturn]]
    auto diagnose() noexcept -> void;

public:
    explicit Vm(Ptr<IRModule> irmod, JitCompiler& jitc)
    : irmodule_(irmod), jitc_(jitc) {}

    auto run() noexcept -> void;
};