#pragma once

class Parser;

class Codegen {
    Parser& parser_;
    Ptr<IRModule> irmodule_;
    uint32_t current_bb_;

    uint16_t reg_counter_ = 0;
    Vec<uint16_t> free_regs_;

    auto alloc_reg() -> uint16_t;
    auto recycle_reg(uint16_t reg) -> void;
    auto reset_reg_scope() -> void;
    
public:
    explicit Codegen(Parser& p)
    : parser_(p){}

    auto create_basic_block() noexcept -> uint32_t;
    auto switch_current_block(uint32_t bb) noexcept -> void;
    auto current_block() noexcept -> uint32_t;

    auto emit_ins(Opcode opcode, uint8_t opnum) noexcept -> void;
    auto emit_external(FFIMetaData data) noexcept -> void;
    auto emit_const_i(int32_t val) noexcept -> void;
    auto emit_const_d(double val) noexcept -> void;
    auto emit_const_s(Str val) noexcept -> void;

    auto emit_irmodule() noexcept -> Result<Ptr<IRModule>, ComptimeError>;
};