#include <tuple>

#include "../models/models.hpp"
#include "../kiz.hpp"
#include "vm.hpp"

namespace kiz {

auto Vm::fetch_two_from_stack_top(
    const std::string& op_name
) -> std::tuple<model::Object*, model::Object*> {
    if (op_stack.size() < 2) {
        std::string err_msg = "OP_" + op_name + ": 操作数栈元素不足（需≥2）";
        std::cout << err_msg << "\n";
        assert(false);
    }
    // 栈顶是右操作数（后压入的），次顶是左操作数（先压入的）
    auto b = fetch_one_from_stack_top();
    auto a = fetch_one_from_stack_top();
    return {a, b};
}

// -------------------------- 算术指令 --------------------------
void Vm::exec_ADD(const Instruction& instruction) {
    DEBUG_OUTPUT("exec add...");
    auto [a, b] = fetch_two_from_stack_top("add");
    DEBUG_OUTPUT("a is " + a->debug_string() + ", b is " + b->debug_string());

    call_method(a, "__add__", new model::List({b}));
    DEBUG_OUTPUT("success to call function");
}

void Vm::exec_SUB(const Instruction& instruction) {
    DEBUG_OUTPUT("exec sub...");
    auto [a, b] = fetch_two_from_stack_top("sub");

    call_method(a, "__sub__", new model::List({b}));
}

void Vm::exec_MUL(const Instruction& instruction) {
    DEBUG_OUTPUT("exec mul...");
    auto [a, b] = fetch_two_from_stack_top("mul");

    call_method(a, "__mul__", new model::List({b}));
}

void Vm::exec_DIV(const Instruction& instruction) {
    DEBUG_OUTPUT("exec div...");
    auto [a, b] = fetch_two_from_stack_top("div");

    call_method(a, "__div__", new model::List({b}));
}

void Vm::exec_MOD(const Instruction& instruction) {
    DEBUG_OUTPUT("exec mod...");
    auto [a, b] = fetch_two_from_stack_top("mod");

    call_method(a, "__mod__", new model::List({b}));
}

void Vm::exec_POW(const Instruction& instruction) {
    DEBUG_OUTPUT("exec pow...");
    auto [a, b] = fetch_two_from_stack_top("pow");

    call_method(a, "__pow__", new model::List({b}));
}

void Vm::exec_NEG(const Instruction& instruction) {
    DEBUG_OUTPUT("exec neg...");
    auto a = fetch_one_from_stack_top();
    call_method(a, "__neg__", new model::List({}));
}

// -------------------------- 比较指令 --------------------------
void Vm::exec_EQ(const Instruction& instruction) {
    DEBUG_OUTPUT("exec eq...");
    auto [a, b] = fetch_two_from_stack_top("eq");

    call_method(a, "__eq__", new model::List({b}));
}

void Vm::exec_GT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec gt...");
    auto [a, b] = fetch_two_from_stack_top("gt");

    call_method(a, "__gt__", new model::List({b}));
}

void Vm::exec_LT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec lt...");
    auto [a, b] = fetch_two_from_stack_top("lt");

    call_method(a, "__lt__", new model::List({b}));
}

// -------------------------- 逻辑指令 --------------------------

// 修正NOT指令（逻辑取反+栈检查）
void Vm::exec_NOT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec not...");
    // 栈检查：确保至少有1个操作数
    if (op_stack.empty()) {
        assert(false && "OP_NOT: 操作数栈元素不足（需≥1）");
    }
    // 弹出栈顶操作数
    auto a = op_stack.top();
    op_stack.pop();
    bool result = !is_true(a);
    op_stack.emplace(new model::Bool(result));
}

void Vm::exec_AND(const Instruction& instruction) {
    DEBUG_OUTPUT("exec and...");
    if (op_stack.size() < 2) {
        assert(false && "OP_AND: 操作数栈元素不足（需≥2）");
    }
    auto [a, b] = fetch_two_from_stack_top("and");

    if (!is_true(a)) {
        op_stack.emplace(a);
    } else {
        op_stack.emplace(b);
    }
}


void Vm::exec_OR(const Instruction& instruction) {
    DEBUG_OUTPUT("exec or...");
    if (op_stack.size() < 2) {
        assert(false && "OP_OR: 操作数栈元素不足（需≥2）");
    }
    auto [a, b] = fetch_two_from_stack_top("or");

    if (is_true(a)) {
        op_stack.emplace(a); // 压回原始对象a
    } else {
        op_stack.emplace(b);
    }
}

void Vm::exec_IS(const Instruction& instruction) {
    DEBUG_OUTPUT("exec is...");
    if (op_stack.size() < 2) {
        assert(false && "OP_IS: 操作数栈元素不足（需≥2）");
    }
    auto [a, b] = fetch_two_from_stack_top("is");

    bool is_same = a == b;
    auto* result = model::load_bool(is_same);
    op_stack.push(result);
}

void Vm::exec_GE(const Instruction& instruction) {
    auto [a, b] = fetch_two_from_stack_top("ge");
    call_method(a, "__eq__", new model::List({b}));
    call_method(a, "__gt__", new model::List({b}));
    auto [gt_result, eq_result] = fetch_two_from_stack_top("ge");

    auto result = model::load_false();
    if (is_true(gt_result) or is_true(eq_result)) {
        result->val = true;
        op_stack.emplace(result);
        return;
    }
    op_stack.emplace(result);
}

void Vm::exec_LE(const Instruction& instruction) {
    auto [a, b] = fetch_two_from_stack_top("le");
    call_method(a, "__eq__", new model::List({b}));
    call_method(a, "__lt__", new model::List({b}));
    auto [lt_result, eq_result] = fetch_two_from_stack_top("le");

    auto result = model::load_false();
    if (is_true(lt_result) or is_true(eq_result)) {
        result->val = true;
        op_stack.emplace(result);
        return;
    }
    op_stack.emplace(result);
}

void Vm::exec_NE(const Instruction& instruction) {
    auto [a, b] = fetch_two_from_stack_top("ne");
    call_method(a, "__eq__", new model::List({b}));
    if (is_true(op_stack.top())) {
        op_stack.pop();
        op_stack.emplace(model::load_false());
        return;
    }
    op_stack.pop();
    op_stack.emplace(model::load_true());
}

void Vm::exec_IN(const Instruction& instruction) {
    auto [item, for_check] = fetch_two_from_stack_top("contains");
    call_method(for_check, "contains", new model::List({item}));
}

}
