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
    list_obj->make_ref();  // List 自身引用计std::to_string
    op_stack.push(list_obj);

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
        // 弹出 value（栈顶第一个是 value）
        model::Object* value = op_stack.top();
        op_stack.pop();
        if (!value) {
            throw NativeFuncError("DictMadeError", "Null value in dictionary entry");
        }
        value->make_ref(); // 增加引用计数

        // 弹出 key（栈顶第二个是 key）
        model::Object* key = fetch_one_from_stack_top();

        key->make_ref(); // 增加引用计数

        // 调用 __hash__ 方法获取哈希值
        call_method(key, "__hash__", new model::List({}));
        model::Object* hash_obj = fetch_one_from_stack_top();

        // 检查哈希值类型
        auto* hashed_int = dynamic_cast<model::Int*>(hash_obj);
        if (!hashed_int) {
            instruction_throw("TypeError", "__hash__ must return an integer");
        }
        assert(hashed_int != nullptr);

        elem_list.emplace_back(hashed_int->val, std::pair{key, value});
    }

    // 创建字典对象
    auto* dict_obj = new model::Dictionary(dep::Dict(elem_list));
    dict_obj->make_ref();
    op_stack.push(dict_obj);
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
}


void Vm::exec_CREATE_OBJECT(const Instruction& instruction) {
    auto obj = new model::Object();
    obj->attrs.insert("__parent__", model::based_obj);
    op_stack.emplace(obj);
}

void Vm::exec_STOP(const Instruction& instruction) {
    DEBUG_OUTPUT("exec stop...");
    running = false;
}

}
