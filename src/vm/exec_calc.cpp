#include <tuple>
#include <cassert>

#include "../models/models.hpp"
#include "../kiz.hpp"
#include "vm.hpp"

namespace kiz {

// -------------------------- 算术指令 --------------------------
void Vm::exec_ADD(const Instruction& instruction) {
    DEBUG_OUTPUT("exec add...");
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();
    DEBUG_OUTPUT("a is " + a->debug_string() + ", b is " + b->debug_string());

    // 创建临时参数列表，调用后释放
    model::List* args_list = model::create_list({b});
    call_method(a, "__add__", args_list);
    args_list->del_ref();

    // 释放使用后的操作数
    a->del_ref();
    b->del_ref();
    DEBUG_OUTPUT("success to call function");
}

void Vm::exec_SUB(const Instruction& instruction) {
    DEBUG_OUTPUT("exec sub...");
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();

    model::List* args_list = model::create_list({b});
    call_method(a, "__sub__", args_list);
    args_list->del_ref();

    a->del_ref();
    b->del_ref();
}

void Vm::exec_MUL(const Instruction& instruction) {
    DEBUG_OUTPUT("exec mul...");
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();

    model::List* args_list = model::create_list({b});
    call_method(a, "__mul__", args_list);
    args_list->del_ref();

    a->del_ref();
    b->del_ref();
}

void Vm::exec_DIV(const Instruction& instruction) {
    DEBUG_OUTPUT("exec div...");
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();

    model::List* args_list = model::create_list({b});
    call_method(a, "__div__", args_list);
    args_list->del_ref();

    a->del_ref();
    b->del_ref();
}

void Vm::exec_MOD(const Instruction& instruction) {
    DEBUG_OUTPUT("exec mod...");
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();

    model::List* args_list = model::create_list({b});
    call_method(a, "__mod__", args_list);
    args_list->del_ref();

    a->del_ref();
    b->del_ref();
}

void Vm::exec_POW(const Instruction& instruction) {
    DEBUG_OUTPUT("exec pow...");
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();

    model::List* args_list = model::create_list({b});
    call_method(a, "__pow__", args_list);
    args_list->del_ref();

    a->del_ref();
    b->del_ref();
}

void Vm::exec_NEG(const Instruction& instruction) {
    DEBUG_OUTPUT("exec neg...");
    auto a = fetch_one_from_stack_top();

    // 无参数，创建空列表
    model::List* args_list = model::create_list({});
    call_method(a, "__neg__", args_list);
    args_list->del_ref();

    a->del_ref();
}

// -------------------------- 比较指令 --------------------------
void Vm::exec_EQ(const Instruction& instruction) {
    DEBUG_OUTPUT("exec eq...");
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();

    model::List* args_list = model::create_list({b});
    call_method(a, "__eq__", args_list);
    args_list->del_ref();

    a->del_ref();
    b->del_ref();
}

void Vm::exec_GT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec gt...");
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();

    model::List* args_list = model::create_list({b});
    call_method(a, "__gt__", args_list);
    args_list->del_ref();

    a->del_ref();
    b->del_ref();
}

void Vm::exec_LT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec lt...");
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();

    model::List* args_list = model::create_list({b});
    call_method(a, "__lt__", args_list);
    args_list->del_ref();

    a->del_ref();
    b->del_ref();
}

// -------------------------- 逻辑指令 --------------------------
void Vm::exec_NOT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec not...");
    // 栈检查：确保至少有1个操作数
    if (op_stack.empty()) {
        assert(false && "OP_NOT: 操作数栈元素不足（需≥1）");
    }
    // 统一使用封装的栈操作
    auto a = fetch_one_from_stack_top();
    bool result = !is_true(a);
    push_to_stack(model::load_bool(result));

    // 释放使用后的操作数
    a->del_ref();
}

void Vm::exec_IS(const Instruction& instruction) {
    DEBUG_OUTPUT("exec is...");
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();
    bool is_same = a == b;

    // 压入比较结果
    push_to_stack(model::load_bool(is_same));
    // 释放使用后的操作数
    a->del_ref();
    b->del_ref();
}

void Vm::exec_GE(const Instruction& instruction) {
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();

    // 调用__eq__方法，释放临时参数
    model::List* eq_args = model::create_list({b});
    call_method(a, "__eq__", eq_args);
    eq_args->del_ref();

    // 调用__gt__方法，释放临时参数
    model::List* gt_args = model::create_list({b});
    call_method(a, "__gt__", gt_args);
    gt_args->del_ref();

    // 统一使用封装的栈操作获取结果
    auto eq_result = fetch_one_from_stack_top();
    auto gt_result = fetch_one_from_stack_top();

    // 压入最终结果
    if (is_true(gt_result) or is_true(eq_result)) {
        push_to_stack(model::load_true());
    } else {
        push_to_stack(model::load_false());
    }

    // 释放所有使用后的对象
    a->del_ref();
    b->del_ref();
    eq_result->del_ref();
    gt_result->del_ref();
}

void Vm::exec_LE(const Instruction& instruction) {
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();

    // 调用__eq__方法
    model::List* eq_args = model::create_list({b});
    call_method(a, "__eq__", eq_args);
    eq_args->del_ref();

    // 调用__lt__方法
    model::List* lt_args = model::create_list({b});
    call_method(a, "__lt__", lt_args);
    lt_args->del_ref();

    // 获取结果
    auto eq_result = fetch_one_from_stack_top();
    auto lt_result = fetch_one_from_stack_top();

    // 压入最终结果
    if (is_true(lt_result) or is_true(eq_result)) {
        push_to_stack(model::load_true());
    } else {
        push_to_stack(model::load_false());
    }

    // 释放所有使用后的对象
    a->del_ref();
    b->del_ref();
    eq_result->del_ref();
    lt_result->del_ref();
}

void Vm::exec_NE(const Instruction& instruction) {
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();

    // 调用__eq__方法
    model::List* eq_args = model::create_list({b});
    call_method(a, "__eq__", eq_args);
    eq_args->del_ref();

    // 获取比较结果
    auto eq_result = fetch_one_from_stack_top();

    // 压入取反结果
    if (is_true(eq_result)) {
        push_to_stack(model::load_false());
    } else {
        push_to_stack(model::load_true());
    }

    // 释放所有使用后的对象
    a->del_ref();
    b->del_ref();
    eq_result->del_ref();
}

void Vm::exec_IN(const Instruction& instruction) {
    auto for_check = fetch_one_from_stack_top();
    auto item = fetch_one_from_stack_top();

    // 调用contains方法，参数为item
    model::List* args_list = model::create_list({item});
    call_method(for_check, "contains", args_list);
    args_list->del_ref();

    // 释放使用后的对象
    for_check->del_ref();
    item->del_ref();
}

} // namespace kiz