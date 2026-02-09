#include <format>

#include "../kiz.hpp"
#include "../opcode/opcode.hpp"
#include "ir_gen.hpp"
#include "../parser/ast.hpp"
#include "../models/models.hpp"

namespace kiz {

model::Module* IRGenerator::gen_mod(
    const std::string& module_name,
    model::CodeObject* module_code
) {
    DEBUG_OUTPUT("code object created with code list len " + std::to_string(module_code->code.size()));
    assert(module_code != nullptr);
    const auto module_obj = new model::Module(
        module_name,
        module_code
    );
    module_obj->make_ref();
    return module_obj;
}


// 处理代码块（生成块内所有语句的IR）
void IRGenerator::gen_block(const BlockStmt* block) {
    if (!block) return;
    for (auto& stmt : block->statements) {
        switch (stmt->ast_type) {
            case AstType::ImportStmt: {
                const auto* import_stmt = dynamic_cast<ImportStmt*>(stmt.get());
                const size_t name_idx = get_or_add_name(curr_names, import_stmt->path);

                curr_code_list.emplace_back(
                    Opcode::IMPORT,
                    std::vector{name_idx},
                    stmt->pos
                );
                break;
            }
            case AstType::AssignStmt: {
                // 变量声明：生成初始化表达式IR + 存储变量指令
                const auto* var_decl = dynamic_cast<AssignStmt*>(stmt.get());
                gen_expr(var_decl->expr.get()); // 生成初始化表达式IR
                const size_t name_idx = get_or_add_name(curr_names, var_decl->name);

                curr_code_list.emplace_back(
                    Opcode::SET_LOCAL,
                    std::vector{name_idx},
                    stmt->pos
                );
                break;
            }
            case AstType::NonlocalAssignStmt: {
                // 变量声明：生成初始化表达式IR + 存储变量指令
                const auto* var_decl = dynamic_cast<NonlocalAssignStmt*>(stmt.get());
                gen_expr(var_decl->expr.get()); // 生成初始化表达式IR
                const size_t name_idx = get_or_add_name(curr_names, var_decl->name);

                curr_code_list.emplace_back(
                    Opcode::SET_NONLOCAL,
                    std::vector<size_t>{name_idx},
                    stmt->pos
                );
                break;
            }

            case AstType::GlobalAssignStmt: {
                // 变量声明：生成初始化表达式IR + 存储变量指令
                const auto* var_decl = dynamic_cast<GlobalAssignStmt*>(stmt.get());
                gen_expr(var_decl->expr.get()); // 生成初始化表达式IR
                const size_t name_idx = get_or_add_name(curr_names, var_decl->name);

                curr_code_list.emplace_back(
                    Opcode::SET_GLOBAL,
                    std::vector<size_t>{name_idx},
                    stmt->pos
                );
                break;
            }
            case AstType::ObjectStmt: {
                const auto* obj_decl = dynamic_cast<ObjectStmt*>(stmt.get());
                const size_t name_idx = get_or_add_name(curr_names, obj_decl->name);

                curr_code_list.emplace_back(
                    Opcode::CREATE_OBJECT,
                    std::vector<size_t>{},
                    stmt->pos
                );

                curr_code_list.emplace_back(
                    Opcode::SET_LOCAL,
                    std::vector{name_idx},
                    stmt->pos
                );

                if (!obj_decl->parent_name.empty()) {
                    const size_t parent_name_idx = get_or_add_name(curr_names, obj_decl->parent_name);

                    curr_code_list.emplace_back(
                        Opcode::LOAD_VAR,
                        std::vector{name_idx},
                        stmt->pos
                    );

                    curr_code_list.emplace_back(
                        Opcode::LOAD_VAR,
                        std::vector{parent_name_idx},
                        stmt->pos
                    );

                    const size_t parent_text_idx = get_or_add_name(curr_names, "__parent__");
                    curr_code_list.emplace_back(
                        Opcode::SET_ATTR,
                        std::vector{parent_text_idx},
                        stmt->pos
                    );
                }

                for (const auto& sub_assign: obj_decl->body->statements) {
                    const auto sub_assign_stmt = dynamic_cast<AssignStmt*>(sub_assign.get());
                    if(sub_assign_stmt == nullptr) {
                        err::error_reporter(file_path, stmt->pos,
                "SyntaxError",
                "Object Statement cannot include other code (only assign statement support)"
                        );
                    }
                    curr_code_list.emplace_back(
                        Opcode::LOAD_VAR,
                        std::vector{name_idx},
                        stmt->pos
                    );

                    assert(sub_assign_stmt->expr.get() != nullptr);
                    gen_expr(sub_assign_stmt->expr.get());
                    const size_t sub_name_idx = get_or_add_name(curr_names, sub_assign_stmt->name);

                    curr_code_list.emplace_back(
                        Opcode::SET_ATTR,
                        std::vector{sub_name_idx},
                        stmt->pos
                    );

                }

                break;
            }
            case AstType::ExprStmt: {
                // 表达式语句：生成表达式IR + 弹出结果（避免栈泄漏）
                auto* expr_stmt = dynamic_cast<ExprStmt*>(stmt.get());
                gen_expr(expr_stmt->expr.get());
                break;
            }
            case AstType::IfStmt:
                gen_if(dynamic_cast<IfStmt*>(stmt.get()));
                break;
            case AstType::ForStmt:
                gen_for(dynamic_cast<ForStmt*>(stmt.get()));
                break;
            case AstType::WhileStmt:
                gen_while(dynamic_cast<WhileStmt*>(stmt.get()));
                break;
            case AstType::TryStmt:
                gen_try(dynamic_cast<TryStmt*>(stmt.get()));
                break;
            case AstType::ReturnStmt: {
                // 返回语句：生成返回值表达式IR + RET指令
                auto* ret_stmt = dynamic_cast<ReturnStmt*>(stmt.get());
                if (ret_stmt->expr) {
                    gen_expr(ret_stmt->expr.get());
                } else {
                    // 无返回值时压入Nil常量
                    auto* nil = model::load_nil();
                    const size_t const_idx = get_or_add_const(nil);
                    curr_code_list.emplace_back(
                        Opcode::LOAD_CONST,
                        std::vector<size_t>{const_idx},
                        stmt->pos
                    );
                }
                curr_code_list.emplace_back(
                    Opcode::RET,
                    std::vector<size_t>{},
                    stmt->pos
                );
                break;
            }
            case AstType::ThrowStmt: {
                auto* throw_stmt = dynamic_cast<ThrowStmt*>(stmt.get());
                gen_expr(throw_stmt->expr.get());
                curr_code_list.emplace_back(
                    Opcode::THROW,
                    std::vector<size_t>{},
                    stmt->pos
                );
                break;
            }
            case AstType::BreakStmt: {
                assert(!block_stack.empty());
                block_stack.top().break_pos.push_back(curr_code_list.size());
                curr_code_list.emplace_back(
                    Opcode::JUMP,
                    std::vector<size_t>{0},
                    stmt->pos
                );
                break;
            }
            case AstType::NextStmt: {
                assert(!block_stack.empty());
                block_stack.top().continue_pos.push_back(curr_code_list.size());
                curr_code_list.emplace_back(
                    Opcode::JUMP,
                    std::vector<size_t>{0},
                    stmt->pos
                );
                break;
            }
            case AstType::SetMemberStmt: {
                // 设置成员：生成对象表达式 -> 生成值表达式 -> 加载属性名 -> SET_ATTR指令
                const auto* set_mem = dynamic_cast<SetMemberStmt*>(stmt.get());
                const auto* get_mem = dynamic_cast<GetMemberExpr*>(set_mem->g_mem.get());
                assert(get_mem != nullptr);
                gen_expr(get_mem->father.get()); // 生成对象IR
                gen_expr(set_mem->val.get());   // 生成值IR

                size_t name_idx = get_or_add_name(curr_names, get_mem->child->name);
                curr_code_list.emplace_back(
                    Opcode::SET_ATTR,
                    std::vector<size_t>{name_idx},
                    stmt->pos
                );
                break;
            }
            case AstType::SetItemStmt: {
                const auto* set_item = dynamic_cast<SetItemStmt*>(stmt.get());
                const auto* get_item = dynamic_cast<GetItemExpr*>(set_item->g_item.get());

                gen_expr(get_item->father.get()); // 生成对象IR
                gen_expr(get_item->params[0].get()); // 生成第一参数(仅支持一个参数)
                gen_expr(set_item->val.get());   // 生成值IR

                curr_code_list.emplace_back(
                    Opcode::SET_ITEM,
                    std::vector<size_t>{},
                    stmt->pos
                );
                break;
            }
            default:
                assert(false && "gen_block: 未处理的语句类型");
        }
    }
}

void IRGenerator::gen_if(IfStmt* if_stmt) {
    assert(if_stmt && "gen_if: if节点为空");
    // 生成条件表达式IR
    gen_expr(if_stmt->condition.get());

    // 生成JUMP_IF_FALSE指令（目标先占位，后续填充）
    size_t jump_if_false_idx = curr_code_list.size();
    curr_code_list.emplace_back(
        Opcode::JUMP_IF_FALSE,
        std::vector<size_t>{0}, // 占位目标索引
        if_stmt->pos
    );

    // 生成then块IR
    gen_block(if_stmt->thenBlock.get());

    // 生成JUMP指令（跳过else块，目标占位）
    size_t jump_else_idx = curr_code_list.size();
    curr_code_list.emplace_back(
        Opcode::JUMP,
        std::vector<size_t>{0}, // 占位目标索引
        if_stmt->pos
    );

    // 填充JUMP_IF_FALSE的目标（else块开始位置）
    curr_code_list[jump_if_false_idx].opn_list[0] = curr_code_list.size();

    // 生成else块IR（存在则生成）
    if (if_stmt->elseBlock) {
        gen_block(if_stmt->elseBlock.get());
    }

    // 填充JUMP的目标（if-else结束位置）
    curr_code_list[jump_else_idx].opn_list[0] = curr_code_list.size();
}

void IRGenerator::gen_while(WhileStmt* while_stmt) {
    assert(while_stmt && "gen_while: while节点为空");
    // 记录循环入口（条件判断开始位置）→ continue跳这里
    size_t loop_entry_idx = curr_code_list.size();

    // 生成循环条件IR
    gen_expr(while_stmt->condition.get());

    // 生成JUMP_IF_FALSE指令（目标：循环结束位置，先占位）
    const size_t jump_if_false_idx = curr_code_list.size();
    curr_code_list.emplace_back(
        Opcode::JUMP_IF_FALSE,
        std::vector<size_t>{0}, // 占位，后续填充为循环结束位置
        while_stmt->pos
    );

    auto loop_info = LoopInfo{{}, {}};
    block_stack.emplace(loop_info);

    // 生成循环体IR
    gen_block(while_stmt->body.get());

    // 生成JUMP指令，跳回循环入口
    curr_code_list.emplace_back(
        Opcode::JUMP,
        std::vector<size_t>{loop_entry_idx},
        while_stmt->pos
    );

    // 填充JUMP_IF_FALSE的目标（循环结束位置 = 当前代码列表长度）
    size_t loop_exit_idx = curr_code_list.size();
    curr_code_list[jump_if_false_idx].opn_list[0] = loop_exit_idx;

    for (const auto break_pos : block_stack.top().break_pos) {
        curr_code_list[break_pos].opn_list[0] = loop_exit_idx;
    }

    for (const auto continue_pos : block_stack.top().continue_pos) {
        curr_code_list[continue_pos].opn_list[0] = loop_entry_idx;
    }

    block_stack.pop();
}

void IRGenerator::gen_for(ForStmt* for_stmt) {
    assert(for_stmt);

    // 生成循环iter IR
    gen_expr(for_stmt->iter.get());

    curr_code_list.emplace_back(
        Opcode::CACHE_ITER,
        std::vector<size_t>{},
        for_stmt->pos
    );

    // 记录循环入口（条件判断开始位置）→ continue跳这里
    size_t loop_entry_idx = curr_code_list.size();

    curr_code_list.emplace_back(
        Opcode::MAKE_LIST,
        std::vector<size_t>{0},
        for_stmt->pos
    );

    curr_code_list.emplace_back(
        Opcode::GET_ITER,
        std::vector<size_t>{},
        for_stmt->pos
    );

    size_t name_idx = get_or_add_name(curr_names, "__next__");

    curr_code_list.emplace_back(
        Opcode::CALL_METHOD,
        std::vector{name_idx},
        for_stmt->pos
    );

    size_t var_name_idx = get_or_add_name(curr_names, for_stmt->item_var_name);
    curr_code_list.emplace_back(
        Opcode::SET_LOCAL,
        std::vector{var_name_idx},
        for_stmt->pos
    );

    curr_code_list.emplace_back(
        Opcode::LOAD_VAR,
        std::vector{var_name_idx},
        for_stmt->pos
    );

    // 生成JUMP_IF_FALSE指令（目标：循环结束位置，先占位）
    const size_t jump_if_false_idx = curr_code_list.size();
    curr_code_list.emplace_back(
        Opcode::JUMP_IF_FINISH_ITER,
        std::vector<size_t>{0}, // 占位，后续填充为循环结束位置
        for_stmt->pos
    );

    auto loop_info = LoopInfo{{}, {}};
    block_stack.emplace(loop_info);

    // 生成循环体IR
    gen_block(for_stmt->body.get());

    // 生成JUMP指令，跳回循环入口
    curr_code_list.emplace_back(
        Opcode::JUMP,
        std::vector{loop_entry_idx},
        for_stmt->pos
    );

    // 填充JUMP_IF_FALSE的目标（循环结束位置 = 当前代码列表长度）
    size_t loop_exit_idx = curr_code_list.size();
    curr_code_list[jump_if_false_idx].opn_list[0] = loop_exit_idx;

    curr_code_list.emplace_back(
        Opcode::POP_ITER,
        std::vector<size_t>{},
        for_stmt->pos
    );

    for (const auto break_pos : block_stack.top().break_pos) {
        curr_code_list[break_pos].opn_list[0] = loop_exit_idx;
    }

    for (const auto continue_pos : block_stack.top().continue_pos) {
        curr_code_list[continue_pos].opn_list[0] = loop_entry_idx;
    }

    block_stack.pop();

}

void IRGenerator::gen_try(TryStmt* try_stmt) {
    assert(try_stmt);

    // 添加TryFrame{catch_start, finally_start}
    size_t try_start_idx = curr_code_list.size();
    curr_code_list.emplace_back(
        Opcode::ENTER_TRY,
        std::vector<size_t>{0, 0},  // 占位[catch_start, finally_start]
        try_stmt->pos
    );

    // 生成 try 块的语句
    gen_block(try_stmt->try_block.get());

    // 标记为可以解决错误
    curr_code_list.emplace_back(Opcode::MARK_HANDLE_ERROR, std::vector<size_t>{}, try_stmt->pos);

    // 跳转到finally
    size_t try_end_idx = curr_code_list.size();
    curr_code_list.emplace_back(
        Opcode::JUMP,
        std::vector<size_t>{0},  // 占位
        try_stmt->pos
    );

    curr_code_list[try_start_idx].opn_list[0] = curr_code_list.size();

    std::vector<size_t> catch_jump_to_finally;
    for (const auto& catch_stmt : try_stmt->catch_blocks) {
        // 加载实际错误
        curr_code_list.emplace_back(
            Opcode::LOAD_ERROR, std::vector<size_t>{}, catch_stmt->pos
        );

        // 加载需捕获的错误类 catch e : Error
        //                           ^^^^^
        gen_expr(catch_stmt->error.get());

        // 判断 需捕获的错误类  是不是 实际错误的原型
        curr_code_list.emplace_back(
            Opcode::IS_CHILD, std::vector<size_t>{}, catch_stmt->pos
        );
        // 如果不是就跳转到下一个catch
        size_t curr_jump_if_false_idx = curr_code_list.size();
        curr_code_list.emplace_back(
            Opcode::JUMP_IF_FALSE, std::vector<size_t>{0}, catch_stmt->pos  // 占位
        );

        // 如果是就继续
        // 标记为可以解决错误
        curr_code_list.emplace_back(Opcode::MARK_HANDLE_ERROR, std::vector<size_t>{}, try_stmt->pos);

        // 加载实际错误
        curr_code_list.emplace_back(
            Opcode::LOAD_ERROR, std::vector<size_t>{}, catch_stmt->pos
        );

        // 储存到变量
        // 加载需捕获的错误类 catch e : Error
        //                      ^^
        const size_t name_idx = get_or_add_name(curr_names, catch_stmt->var_name);
        curr_code_list.emplace_back(
            Opcode::SET_LOCAL, std::vector{name_idx}, catch_stmt->pos
        );


        // 生成 catch 块的语句
        gen_block(catch_stmt->catch_block.get());


        // 处理后跳转到finally
        catch_jump_to_finally.emplace_back(curr_code_list.size());
        curr_code_list.emplace_back(
            Opcode::JUMP, std::vector<size_t>{0}, catch_stmt->pos  // 占位
        );

        size_t end_catch_idx = curr_code_list.size();
        curr_code_list[curr_jump_if_false_idx].opn_list[0] = end_catch_idx;
    }

    // 生成 finally 块的语句
    const size_t finally_start_idx = curr_code_list.size();
    if (try_stmt->finally_block) {
        gen_block(try_stmt->finally_block.get());
    }

    // 回填 finally 块开始处
    curr_code_list[try_start_idx].opn_list[1] = finally_start_idx;
    curr_code_list[try_end_idx].opn_list[0] = finally_start_idx;

    for (auto pos : catch_jump_to_finally) {
        curr_code_list[pos].opn_list[0] = finally_start_idx;
    }


    size_t skip_rethrow_idx = curr_code_list.size();
    curr_code_list.emplace_back(
        Opcode::JUMP_IF_FINISH_HANDLE_ERROR, std::vector<size_t>{0}, try_stmt->pos  // 占位
    );
    curr_code_list.emplace_back( Opcode::LOAD_ERROR, std::vector<size_t>{}, try_stmt->pos);
    curr_code_list.emplace_back(Opcode::THROW, std::vector<size_t>{}, try_stmt->pos);

    curr_code_list[skip_rethrow_idx].opn_list[0] = curr_code_list.size();
}

}
