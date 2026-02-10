
#include <cassert>

#include "../models/models.hpp"
#include "vm.hpp"
#include "opcode/opcode.hpp"

namespace kiz {

// -------------------------- 函数调用/返回 --------------------------
void Vm::exec_CALL(const Instruction& instruction) {
    DEBUG_OUTPUT("exec call...");

    // 栈中至少需要 2 个元素
    if (op_stack.size() < 2) {
        assert(false && "CALL: 操作数栈元素不足（需≥2：函数对象 + 参数列表）");
    }
    if (call_stack.empty()) {
        assert(false && "CALL: 无活跃调用帧");
    }

    // 弹出栈顶元素 : 函数对象
    model::Object* func_obj = fetch_one_from_stack_top();
    func_obj->make_ref();  // 临时持有函数对象，避免中途被释放

    // 弹出栈顶-1元素 : 参数列表
    model::Object* args_obj = fetch_one_from_stack_top();

    DEBUG_OUTPUT("弹出函数对象: " + func_obj->debug_string());
    DEBUG_OUTPUT("弹出参数列表: " + args_obj->debug_string());
    handle_call(func_obj, args_obj);

    // ==| IMPORTANT |==
    func_obj->del_ref(); // 修复：释放临时持有引用
}

void Vm::exec_CALL_METHOD(const Instruction& instruction) {
    DEBUG_OUTPUT("exec call method...");

    // 弹出栈顶元素 : 源对象
    auto obj = fetch_one_from_stack_top();
    obj->make_ref();

    // 弹出栈顶-1元素 : 参数列表
    model::Object* args_obj = fetch_one_from_stack_top();

    DEBUG_OUTPUT("弹出对象: " + obj->debug_string());
    DEBUG_OUTPUT("弹出参数列表: " + args_obj->debug_string());

    size_t name_idx = instruction.opn_list[0];
    CallFrame* curr_frame = call_stack.back().get();

    if (name_idx >= curr_frame->code_object->names.size()) {
        assert(false && "GET_ATTR: 属性名索引超出范围");
    }
    std::string attr_name = curr_frame->code_object->names[name_idx];

    auto func_obj = get_attr(obj, attr_name);
    func_obj->make_ref();

    DEBUG_OUTPUT("获取函数对象: " + func_obj->debug_string());
    handle_call(func_obj, args_obj, obj);

    func_obj->del_ref();
    obj->del_ref();
}

void Vm::exec_RET(const Instruction& instruction) {
    DEBUG_OUTPUT("exec ret...");
    // 兼容顶层调用帧返回
    if (call_stack.size() < 2) {
        if (!op_stack.empty()) {
            auto top_val = fetch_one_from_stack_top();
            if (top_val) top_val->del_ref();
        }
        call_stack.pop_back();
        return;
    }

    auto curr_frame = call_stack.back();
    call_stack.pop_back();

    CallFrame* caller_frame = call_stack.back().get();
    model::Object* return_val = model::unique_nil;

    if (!op_stack.empty()) {
        return_val = fetch_one_from_stack_top(); // 释放持有
    }

    caller_frame->pc = curr_frame->return_to_pc;
    push_to_stack(return_val);
}
}
