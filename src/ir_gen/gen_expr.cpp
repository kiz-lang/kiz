#include "../opcode/opcode.hpp"
#include "ir_gen.hpp"
#include "../parser/ast.hpp"
#include "../models/models.hpp"

namespace kiz {

void IRGenerator::gen_expr(Expr* expr) {
    if(!expr) throw KizStopRunningSignal("gen_expr: null expr node");
    switch (expr->ast_type) {
        case AstType::NumberExpr: {
            // 生成LOAD_CONST指令（加载字面量常量）
            auto const_obj = make_int_obj(dynamic_cast<NumberExpr*>(expr));
            size_t const_idx = get_or_add_const(const_obj);
            curr_code_list.emplace_back(
                Opcode::LOAD_CONST,
                std::vector{const_idx},
                expr->pos
            );
            break;
        }
        case AstType::StringExpr: {
            // 生成LOAD_CONST指令（加载字面量常量）
            auto const_obj = make_string_obj(dynamic_cast<StringExpr*>(expr));
            size_t const_idx = get_or_add_const(const_obj);
            curr_code_list.emplace_back(
                Opcode::LOAD_CONST,
                std::vector{const_idx},
                expr->pos
            );
            break;
        }
        case AstType::DecimalExpr: {
            // 生成LOAD_CONST指令（加载字面量常量）
            auto const_obj = make_decimal_obj(dynamic_cast<DecimalExpr*>(expr));
            size_t const_idx = get_or_add_const(const_obj);
            curr_code_list.emplace_back(
                Opcode::LOAD_CONST,
                std::vector{const_idx},
                expr->pos
            );
            break;
        }
        case AstType::IdentifierExpr: {
            // 标识符：生成LOAD_VAR指令（加载变量值）
            const auto* ident = dynamic_cast<IdentifierExpr*>(expr);
            const size_t name_idx = get_or_add_name(curr_names, ident->name);
            curr_code_list.emplace_back(
                Opcode::LOAD_VAR,
                std::vector<size_t>{name_idx},
                expr->pos
            );
            break;
        }
        case AstType::BinaryExpr: {
            // 二元运算：生成左表达式 -> 右表达式 -> 运算指令
            const auto* bin_expr = dynamic_cast<BinaryExpr*>(expr);
            if (bin_expr->op == "and"){
                gen_expr(bin_expr->left.get());  // 左操作数

                curr_code_list.emplace_back(Opcode::COPY_TOP, std::vector<size_t>{}, expr->pos);

                size_t jump_if_false_idx = curr_code_list.size();
                curr_code_list.emplace_back(Opcode::JUMP_IF_FALSE, std::vector<size_t>{0}, expr->pos);

                gen_expr(bin_expr->right.get()); // 右操作数（栈中顺序：左在下，右在上）
                curr_code_list[jump_if_false_idx].opn_list[0] = curr_code_list.size();
                break;
            }
            if (bin_expr->op == "or") {
                gen_expr(bin_expr->left.get());  // 左操作数

                curr_code_list.emplace_back(Opcode::COPY_TOP, std::vector<size_t>{}, expr->pos);

                curr_code_list.emplace_back(Opcode::OP_NOT, std::vector<size_t>{}, expr->pos);
                size_t jump_if_false_idx = curr_code_list.size();
                curr_code_list.emplace_back(Opcode::JUMP_IF_FALSE, std::vector<size_t>{0}, expr->pos);

                gen_expr(bin_expr->right.get()); // 右操作数（栈中顺序：左在下，右在上）
                curr_code_list[jump_if_false_idx].opn_list[0] = curr_code_list.size();
                break;
            }
            gen_expr(bin_expr->left.get());  // 左操作数

            gen_expr(bin_expr->right.get()); // 右操作数（栈中顺序：左在下，右在上）

            // 映射运算符到 opcode
            Opcode opc;
            if (bin_expr->op == "+") opc = Opcode::OP_ADD;
            else if (bin_expr->op == "-") opc = Opcode::OP_SUB;
            else if (bin_expr->op == "*") opc = Opcode::OP_MUL;
            else if (bin_expr->op == "/") opc = Opcode::OP_DIV;
            else if (bin_expr->op == "%") opc = Opcode::OP_MOD;
            else if (bin_expr->op == "^") opc = Opcode::OP_POW;

            else if (bin_expr->op == "==") opc = Opcode::OP_EQ;
            else if (bin_expr->op == ">=") opc = Opcode::OP_GE;
            else if (bin_expr->op == "<=") opc = Opcode::OP_LE;
            else if (bin_expr->op == "!=") opc = Opcode::OP_NE;
            else if (bin_expr->op == ">") opc = Opcode::OP_GT;
            else if (bin_expr->op == "<") opc = Opcode::OP_LT;

            else if (bin_expr->op == "is") opc = Opcode::OP_IS;
            else if (bin_expr->op == "in") opc = Opcode::OP_IN;

            else assert(false);

            curr_code_list.emplace_back(
                opc,
                std::vector<size_t>{},
                expr->pos
            );
            break;
        }
        case AstType::UnaryExpr: {
            // 一元运算：生成操作数 -> 运算指令
            auto* unary_expr = dynamic_cast<UnaryExpr*>(expr);
            gen_expr(unary_expr->operand.get());

            Opcode opc;
            if (unary_expr->op == "-") opc = Opcode::OP_NEG;
            else if (unary_expr->op == "not") opc = Opcode::OP_NOT;
            else assert(false);

            curr_code_list.emplace_back(
                opc,
                std::vector<size_t>{},
                expr->pos
            );
            break;
        }
        case AstType::CallExpr:
            DEBUG_OUTPUT("gen fn call...");
            gen_fn_call(dynamic_cast<CallExpr*>(expr));
            break;
        case AstType::DictExpr:
            gen_dict(dynamic_cast<DictExpr*>(expr));
            break;
        case AstType::ListExpr: {
            auto list_expr = dynamic_cast<ListExpr*>(expr);
            for (const auto& e: list_expr->elements) {
                gen_expr(e.get());
            }
            // 生成 OP_MAKE_LIST 指令
            curr_code_list.emplace_back(
                Opcode::MAKE_LIST,
                std::vector{list_expr->elements.size()},
                expr->pos
           );
            break;
        }
        case AstType::GetMemberExpr: {
            // 获取成员：生成对象表达式 -> 加载属性名 -> GET_ATTR指令
            auto* get_mem = dynamic_cast<GetMemberExpr*>(expr);
            gen_expr(get_mem->father.get()); // 生成对象IR
            size_t name_idx = get_or_add_name(curr_names, get_mem->child->name);
            curr_code_list.emplace_back(
                Opcode::GET_ATTR,
                std::vector<size_t>{name_idx},
                expr->pos
            );
            break;
        }
        case AstType::GetItemExpr: {
            auto get_mem_expr = dynamic_cast<GetItemExpr*>(expr);
            size_t arg_count = get_mem_expr->params.size();

            for (auto& arg : get_mem_expr->params) {
                gen_expr(arg.get());
            }

            // 生成 OP_MAKE_LIST 指令：将栈顶 arg_count 个元素打包成参数列表，压回栈
            curr_code_list.emplace_back(
                Opcode::MAKE_LIST,
                std::vector{arg_count},
                get_mem_expr->pos
            );

            gen_expr(get_mem_expr->father.get());

            curr_code_list.emplace_back(
                Opcode::GET_ITEM,
                std::vector<size_t>{},
                get_mem_expr->pos
            );
            break;
        }
        case AstType::FuncDeclExpr: {
            // 匿名函数：同普通函数声明，生成函数对象后加载
            auto* lambda = dynamic_cast<FnDeclExpr*>(expr);
            // 临时保存当前模块级代码容器
            auto save_code = curr_code_list;
            auto save_names = curr_names;
            auto save_const = curr_consts;

            // 初始化lambda代码容器
            curr_code_list.clear();
            curr_names.clear();
            curr_consts.clear();

            // 添加参数到lambda变量表
            for (const auto& param : lambda->params) {
                get_or_add_name(curr_names, param);
            }
            // 生成lambda函数体
            gen_block(lambda->body.get());
            // 确保lambda有返回值（无显式返回则返回Nil）
            if (curr_code_list.empty() || curr_code_list.back().opc != Opcode::RET) {
                const auto nil = model::load_nil();
                const size_t nil_idx = get_or_add_const(nil);
                curr_code_list.emplace_back(
                    Opcode::LOAD_CONST,
                    std::vector{nil_idx},
                    expr->pos
                );
                curr_code_list.emplace_back(
                    Opcode::RET,
                    std::vector<size_t>{},
                    expr->pos
                );
            }

            const auto code_obj = new model::CodeObject(
                curr_code_list,
                curr_names
            );

            // 生成lambda函数体IR
            const auto lambda_fn = new model::Function(
                lambda->name.empty() ? "<lambda>" : lambda->name,
                code_obj,
                lambda->params.size()
            );
            lambda_fn->has_rest_params = lambda->has_rest_params;

            // 恢复模块级代码容器
            curr_code_list = save_code;
            curr_names = save_names;
            curr_consts = save_const;

            // 加载lambda函数对象
            const size_t fn_const_idx = get_or_add_const(lambda_fn);
            curr_code_list.emplace_back(
                Opcode::LOAD_CONST,
                std::vector{fn_const_idx},
                expr->pos
            );
            break;
        }
        case AstType::NilExpr : {
            const auto nil = model::load_nil();
            const size_t nil_idx = get_or_add_const(nil);
            curr_code_list.emplace_back(
                Opcode::LOAD_CONST,
                std::vector{nil_idx},
                expr->pos
            );
            break;
        }
        case AstType::BoolExpr : {
            const auto bool_ast = dynamic_cast<BoolExpr*>(expr);
            assert(bool_ast!=nullptr);
            const auto bool_obj = model::load_bool(bool_ast->val);
            const size_t bool_idx = get_or_add_const(bool_obj);
            curr_code_list.emplace_back(
                Opcode::LOAD_CONST,
                std::vector{bool_idx},
                expr->pos
            );
            break;
        }
        default:
            assert(false && "gen_expr: 未处理的表达式类型");
    }
}

void IRGenerator::gen_fn_call(CallExpr* call_expr) {
    assert(call_expr && "gen_fn_call: 函数调用节点为空");
    size_t arg_count = call_expr->args.size();

    // 生成所有参数的IR，最终打包成List（与原逻辑一致）
    for (auto& arg : call_expr->args) {
        gen_expr(arg.get());
    }

    // 生成 OP_MAKE_LIST 指令：将栈顶 arg_count 个元素打包成参数列表，压回栈
    curr_code_list.emplace_back(
        Opcode::MAKE_LIST,
        std::vector<size_t>{arg_count},
        call_expr->pos
    );

    // 判断 callee 是否为 GetMemberExpr
    if (auto* member_expr = dynamic_cast<GetMemberExpr*>(call_expr->callee.get())) {
        gen_expr(member_expr->father.get()); 

        // 获取方法名的字符串常量池索引
        const std::string& method_name = member_expr->child->name;
        size_t method_name_idx = get_or_add_name(curr_names, method_name); 

        // 生成 CALL_METHOD 指令：操作数为 方法名索引 + 参数个数（用于校验）
        curr_code_list.emplace_back(
            Opcode::CALL_METHOD,
            std::vector<size_t>{method_name_idx, arg_count},
            call_expr->pos
        );
    } else {
        // 普通函数调用：生成函数对象IR → 生成 CALL 指令
        gen_expr(call_expr->callee.get());
        curr_code_list.emplace_back(
            Opcode::CALL,
            std::vector<size_t>{arg_count},
            call_expr->pos
        );
    }
}

void IRGenerator::gen_dict(DictExpr* expr) {
    assert(expr != nullptr);
    // 处理字典键值对
    for (auto& [key, val_expr] : expr->elements) {
        gen_expr(key.get());
        gen_expr(val_expr.get());
    }

    size_t dict_size = expr->elements.size();
    curr_code_list.emplace_back(
        Opcode::MAKE_DICT,
        std::vector{dict_size},
        expr->pos
    );
}

}
