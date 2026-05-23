#pragma once

class Codegen {
    Parser& parser_;
    Ptr<IRModule> irmodule_;
    Ptr<BasicBlock> cur_bb_;

    uint16_t reg_counter_ = 0;
    Vec<uint16_t> free_regs_;

    auto alloc_reg() -> uint16_t;
    auto recycle_reg(uint16_t reg) -> void;
    auto reset_reg_scope() -> void;
    
public:
    explicit Codegen(Parser& p)
    : parser_(p){}

    auto create_basic_block() noexcept -> BasicBlock&;
    auto switch_current_block(BasicBlock& bb) noexcept -> void;
    auto current_block() noexcept;
    auto push_ins(Opcode opcode, uint8_t opnum) noexcept -> void;
    auto push_const(ConstVal val) noexcept -> void;
    auto emit_irmodule() noexcept -> Result<Ptr<IRModule>, ComptimeError>;
};