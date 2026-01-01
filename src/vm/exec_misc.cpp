
#include <cassert>

#include "models.hpp"
#include "vm.hpp"
#include "builtins/include/builtin_functions.hpp"

namespace kiz {

// -------------------------- 制作列表 --------------------------
void Vm::exec_MAKE_LIST(const Instruction& instruction) {
    DEBUG_OUTPUT("exec make_list...");

    // 校验：操作数必须包含“要打包的元素个数”
    if (instruction.opn_list.empty()) {
        assert(false && "MAKE_LIST: 无元素个数参数");
    }
    size_t elem_count = instruction.opn_list[0];

    // 校验：栈中元素个数 ≥ 要打包的个数
    if (op_stack.size() < elem_count) {
        assert(false && ("MAKE_LIST: 栈元素不足（需" + std::to_string(elem_count) +
                        "个，实际" + std::to_string(op_stack.size()) + "个）").c_str());
    }

    // 弹出栈顶 elem_count 个元素（栈是LIFO，弹出顺序是 argN → arg2 → arg1）
    std::vector<model::Object*> elem_list;
    elem_list.reserve(elem_count);  // 预分配空间，避免扩容
    for (size_t i = 0; i < elem_count; ++i) {
        model::Object* elem = op_stack.top();
        op_stack.pop();

        // 校验：元素不能为 nullptr
        if (elem == nullptr) {
            assert(false && ("MAKE_LIST: 第" + std::to_string(i) + "个元素为nil（非法）").c_str());
        }

        // 关键：List 要持有元素的引用，所以每个元素 make_ref（引用计数+1）
        elem->make_ref();
        elem_list.push_back(elem);
    }

    // 反转元素顺序（恢复原参数顺序：arg1 → arg2 → ... → argN）
    std::reverse(elem_list.begin(), elem_list.end());

    // 创建 List 对象，压入栈
    auto* list_obj = new model::List(elem_list);
    list_obj->make_ref();  // List 自身引用计std::to_stringstd::to_string((数+1
    op_stack.push(list_obj);

    DEBUG_OUTPUT("make_list: 打包 " + std::to_string(elem_count) + " 个元素为 List，压栈成功");
}

void Vm::exec_TRY_START(const Instruction& instruction) {
    size_t catch_start = instruction.opn_list[0];
    call_stack.back()->try_blocks.emplace_back(catch_start);
}

void Vm::exec_TRY_END(const Instruction& instruction) {
    call_stack.back()->try_blocks.pop_back();

    const size_t end_catch_pc = instruction.opn_list[0];
    call_stack.back()->pc = end_catch_pc;
}

void Vm::exec_CLEAN_ERROR(const Instruction& instruction) {
    curr_error = nullptr;
}

void Vm::exec_IMPORT(const Instruction& instruction) {
    size_t name_idx = instruction.opn_list[0];
    std::string name = call_stack.back()->code_object->names[name_idx];
    // todo

}

void Vm::exec_LOAD_ERROR(const Instruction& instruction) {
    DEBUG_OUTPUT("loading curr error" + curr_error->to_string());
    op_stack.push(curr_error);
}

void Vm::exec_SET_ERROR(const Instruction& instruction) {
    auto a = op_stack.top();
    op_stack.pop();
    auto a_err = dynamic_cast<model::Error*>(a);
    curr_error = a_err;
}


void Vm::exec_IS_INSTANCE(const Instruction& instruction) {
    auto [a, b] = fetch_two_from_stack_top("is instance");
    op_stack.emplace(builtin::check_based_object(a, b));
}

// -------------------------- 跳转指令 --------------------------
void Vm::exec_JUMP(const Instruction& instruction) {
    DEBUG_OUTPUT("exec jump...");
    if (instruction.opn_list.empty()) {
        assert(false && "JUMP: 无目标pc索引");
    }
    size_t target_pc = instruction.opn_list[0];
    CallFrame* curr_frame = call_stack.back().get();
    if (target_pc > curr_frame->code_object->code.size()) {
        assert(false && "JUMP: 目标pc超出字节码范围");
    }
    call_stack.back()->pc = target_pc;
}

void Vm::exec_JUMP_IF_FALSE(const Instruction& instruction) {
    DEBUG_OUTPUT("exec jump_if_false...");
    // 检查
    if (op_stack.empty()) assert(false && "JUMP_IF_FALSE: 操作数栈空");
    if (instruction.opn_list.empty()) assert(false && "JUMP_IF_FALSE: 无目标pc");

    // 取出条件值
    model::Object* cond = op_stack.top();
    op_stack.pop();
    const size_t target_pc = instruction.opn_list[0];

    bool need_jump = check_obj_is_true(cond) ? false: true;

    if (need_jump) {
        // 跳转逻辑
        DEBUG_OUTPUT("need jump");
        CallFrame* curr_frame = call_stack.back().get();
        if (target_pc > curr_frame->code_object->code.size()) {
            cond->del_ref();
            assert(false && "JUMP_IF_FALSE: 目标pc超出范围");
        }
        DEBUG_OUTPUT("JUMP_IF_FALSE: 跳转至 PC=" + std::to_string(target_pc));
        call_stack.back()->pc = target_pc;
    } else {
        call_stack.back()->pc++;
    }

    // 释放条件值引用
    cond->del_ref();
}

// -------------------------- 异常处理 --------------------------
void Vm::exec_THROW(const Instruction& instruction) {
    DEBUG_OUTPUT("exec throw...");
    model::Object* top = op_stack.top();
    op_stack.pop();

    throw_error(top);
}

// -------------------------- 栈操作 --------------------------
void Vm::exec_POP_TOP(const Instruction& instruction) {
    DEBUG_OUTPUT("exec pop_top...");
    if (op_stack.empty()) {
        assert(false && "POP_TOP: 操作数栈为空");
    }

    model::Object* top = op_stack.top();
    op_stack.pop();

    if (top == nullptr) {
        assert(false && "POP_TOP: 栈顶元素为nil（非法）");
        return;
    }

    // 临时打印结果（验证是否正确）
    DEBUG_OUTPUT("pop_top: 弹出结果 = " + top->to_string());

    top->del_ref();
}

void Vm::exec_SWAP(const Instruction& instruction) {
    DEBUG_OUTPUT("exec swap...");
    if (op_stack.size() < 2) {
        assert(false && "SWAP: 操作数栈元素不足（需≥2）");
    }
    model::Object* a = op_stack.top();
    op_stack.pop();
    model::Object* b = op_stack.top();
    op_stack.pop();
    op_stack.push(a);
    op_stack.push(b);
}


void Vm::exec_COPY_TOP(const Instruction& instruction) {
    DEBUG_OUTPUT("exec copy_top...");
    if (op_stack.empty()) {
        assert(false && "COPY_TOP: 操作数栈为空");
    }
    model::Object* top = op_stack.top();
    top->make_ref();
    op_stack.emplace(top);
}

void Vm::exec_STOP(const Instruction& instruction) {
    DEBUG_OUTPUT("exec stop...");
    running = false;
}

}
