/**
 * @file ir_gen.cpp
 * @brief 中间代码生成器（IR Generator）核心实现
 * 从AST生成IR
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "ir_gen.hpp"
#include "../parser/ast.hpp"
#include "../models/models.hpp"
#include <algorithm>
#include <cassert>

#include "../kiz.hpp"
#include "../vm/vm.hpp"
#include "../opcode/opcode.hpp"

namespace kiz {

size_t IRGenerator::get_or_add_name(std::vector<std::string>& names, const std::string& name) {
    auto it = std::find(names.begin(), names.end(), name);
    if (it != names.end()) {
        return std::distance(names.begin(), it);
    }
    names.emplace_back(name);
    return names.size() - 1;
}

// 辅助函数：获取常量在curr_const中的索引（不存在则添加）
size_t IRGenerator::get_or_add_const(model::Object* obj) {
    const auto it = std::find(Vm::const_pool.begin(), Vm::const_pool.end(), obj);
    if (it != Vm::const_pool.end()) {
        return std::distance(Vm::const_pool.begin(), it);
    }
    obj->make_ref();
    Vm::const_pool.emplace_back(obj);
    return Vm::const_pool.size() - 1;
}

model::CodeObject* IRGenerator::gen(std::unique_ptr<BlockStmt> ast_into) {
    ast = std::move(ast_into);
    DEBUG_OUTPUT("generating...");
    // 检查AST根节点有效性（默认模块根为BlockStmt）
    assert(ast && ast->ast_type == AstType::BlockStmt && "gen: AST根节点非BlockStmt");
    const auto* root_block = ast.get();

    // 初始化模块级代码容器
    curr_code_list.clear();
    curr_names.clear();
    curr_consts.clear();

    // 处理模块顶层节点
    gen_block(root_block);

    // std::cout << "== IR Result ==" << std::endl;
    // size_t i = 0;
    // for (const auto& inst : curr_code_list) {
    //     std::string opn_text;
    //     for (auto opn : inst.opn_list) {
    //         opn_text += std::to_string(opn) + ",";
    //     }
    //     std::cout << i << ":" << opcode_to_string(inst.opc) << " " << opn_text << std::endl;
    //     ++i;
    // }
    // std::cout << "== End ==" << std::endl;

    auto code = new model::CodeObject(
        curr_code_list,
        curr_names
    );
    code->make_ref();
    return code;
}

model::CodeObject* IRGenerator::make_code_obj() const {
    DEBUG_OUTPUT("making code object...");
    // 复制常量池（管理引用计数）

    DEBUG_OUTPUT("make code obj : ir result");
    for (const auto& inst : curr_code_list) {
        DEBUG_OUTPUT(opcode_to_string(inst.opc));
    }

    const auto code_obj = new model::CodeObject(
        curr_code_list, curr_names
    );
    code_obj->make_ref();
    return code_obj;
}

model::Int* IRGenerator::make_int_obj(const NumberExpr* num_expr) {
    DEBUG_OUTPUT("making int object...");
    assert(num_expr && "make_int_obj: 数字节点为空");
    auto the_num = dep::BigInt(num_expr->value);
    if (the_num >= 0 and the_num < 201) {
        auto obj = Vm::small_int_pool[the_num.to_unsigned_long_long()];
        obj->make_ref();
        return obj;
    }

    auto int_obj = new model::Int( the_num );
    int_obj->make_ref();
    return int_obj;
}


model::Decimal* IRGenerator::make_decimal_obj(const DecimalExpr* dec_expr) {
    DEBUG_OUTPUT("making rational object...");
    auto decimal_str = dep::Decimal(dec_expr->value);
    auto decimal_obj = new model::Decimal(decimal_str);
    decimal_obj->make_ref();
    return decimal_obj;
}

model::String* IRGenerator::make_string_obj(const StringExpr* str_expr) {
    DEBUG_OUTPUT("making string object...");
    assert(str_expr && "make_string_obj: 字符串节点为空");
    auto str_obj = new model::String(str_expr->value);
    str_obj->make_ref();
    return str_obj;
}

} // namespace kiz