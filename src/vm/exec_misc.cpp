#include <cassert>

#include "../models/models.hpp"
#include "vm.hpp"
#include "builtins/include/builtin_functions.hpp"
#include "ir_gen/ir_gen.hpp"
#include "lexer/lexer.hpp"
#include "util/src_manager.hpp"

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
        model::Object* elem = fetch_one_from_stack_top();

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
    push_to_stack(list_obj);

    DEBUG_OUTPUT("make_list: 打包 " + std::to_string(elem_count) + " 个元素为 List，压栈成功");
}

void Vm::exec_MAKE_DICT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec make_dict...");

    size_t elem_count = instruction.opn_list[0];
    const size_t total_elems = elem_count * 2;

    // 校验：栈中元素个数 ≥ 要打包的个数
    if (op_stack.size() < total_elems) {
        assert(false && "Stack underflow in MAKE_DICT: insufficient elements");
    }

    // 栈中顺序是 [key1, val1, key2, val2,...]（栈底→栈顶）
    std::vector<std::pair<
        dep::BigInt, std::pair< model::Object*, model::Object* >
    >> elem_list;
    elem_list.reserve(elem_count);

    for (size_t i = 0; i < elem_count; ++i) {
        model::Object* value = fetch_one_from_stack_top();
        if (!value) {
            throw NativeFuncError("DictMadeError", "Null value in dictionary entry");
        }
        value->make_ref();

        model::Object* key = fetch_one_from_stack_top();
        key->make_ref();

        // 修使用create_list创建临时参数，保存指针以便释放
        model::List* hash_args = model::cast_to_list(model::create_list({}));
        call_method(key, "__hash__", hash_args);
        model::Object* hash_obj = fetch_one_from_stack_top();

        auto* hashed_int = dynamic_cast<model::Int*>(hash_obj);
        if (!hashed_int) {
            // 异常时提前释放临时对象，避免泄漏
            // ==| IMPORTANT |==
            hash_args->del_ref();
            hash_obj->del_ref();
            key->del_ref();
            value->del_ref();
            instruction_throw("TypeError", "__hash__ must return an integer");
        }
        assert(hashed_int != nullptr);

        elem_list.emplace_back(hashed_int->val, std::pair{key, value});

        // 释放临时参数和哈希对象
        hash_args->del_ref();
        hash_obj->del_ref();
    }

    auto* dict_obj = new model::Dictionary(dep::Dict(elem_list));
    push_to_stack(dict_obj);
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
    model::Object* cond =fetch_one_from_stack_top();
    const size_t target_pc = instruction.opn_list[0];

    bool need_jump = is_true(cond) ? false: true;

    if (need_jump) {
        // 跳转逻辑
        DEBUG_OUTPUT("need jump");
        CallFrame* curr_frame = call_stack.back().get();
        if (target_pc > curr_frame->code_object->code.size()) {
            assert(false && "JUMP_IF_FALSE: 目标pc超出范围");
        }
        DEBUG_OUTPUT("JUMP_IF_FALSE: 跳转至 PC=" + std::to_string(target_pc));
        call_stack.back()->pc = target_pc;
    } else {
        call_stack.back()->pc++;
    }
    // ==| IMPORTANT |==
    cond->del_ref();
}


void Vm::exec_CREATE_OBJECT(const Instruction& instruction) {
    auto obj = new model::Object();
    obj->make_ref(); // 创建即计数
    obj->attrs.insert("__parent__", model::based_obj);
    push_to_stack(obj); // push自动make_ref()，计数变为2，栈释放后回到1
}

void Vm::exec_STOP(const Instruction& instruction) {
    DEBUG_OUTPUT("exec stop...");
    running = false;
}

void Vm::exec_CACHE_ITER(const Instruction& instruction) {
    assert(!op_stack.empty());
    auto iter = op_stack.top();
    iter->make_ref();
    assert(iter != nullptr);
    assert(!call_stack.empty());

    call_stack.back().get()->iters.push_back(iter);
}

void Vm::exec_GET_ITER(const Instruction& instruction) {
    assert(!call_stack.empty());
    assert(!call_stack.back().get()->iters.empty());

    push_to_stack(
        call_stack.back().get()->iters.back()
    );
}

void Vm::exec_POP_ITER(const Instruction& instruction) {
    assert(!call_stack.empty());
    auto curr_frame = call_stack.back().get();
    assert(!curr_frame->iters.empty());

    auto iter_obj = curr_frame->iters.back();
    iter_obj->del_ref();
    curr_frame->iters.pop_back();
}

void Vm::exec_JUMP_IF_FINISH_ITER(const Instruction& instruction) {
    auto obj = fetch_one_from_stack_top();
    size_t target_pc = instruction.opn_list[0];
    if (obj == model::stop_iter_signal) {
        call_stack.back()->pc = target_pc;
        obj->del_ref(); // 修复：释放使用后的obj
        return;
    }
    call_stack.back()->pc ++;
    obj->del_ref(); // 修复：释放使用后的obj
}

}
