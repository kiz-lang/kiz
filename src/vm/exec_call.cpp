
#include <cassert>

#include "../models/models.hpp"
#include "vm.hpp"
#include "op_code/opcode.hpp"

namespace kiz {

bool Vm::is_true(model::Object* obj) {
    if (const auto bool_obj = dynamic_cast<const model::Bool*>(obj)) {
        return bool_obj->val==true;
    }
    if (dynamic_cast<const model::Nil*>(obj)) {
        return false;
    }

    call_method(obj, "__bool__", new model::List({}));
    auto result = fetch_one_from_stack_top();
    return is_true(result);
}


void Vm::handle_call(model::Object* func_obj, model::Object* args_obj, model::Object* self=nullptr){
    assert(func_obj != nullptr);
    assert(args_obj != nullptr);
    auto* args_list = dynamic_cast<model::List*>(args_obj);

    if (!args_list) {
        assert(false && "CALL: 栈顶-1元素非List类型（参数必须封装为列表）");
    }
    DEBUG_OUTPUT("start to call function");

    // 分类型处理函数调用（Function / NativeFunction）
    if (const auto* cpp_func = dynamic_cast<model::NativeFunction*>(func_obj)) {
        // -------------------------- 处理 NativeFunction 调用 --------------------------
        DEBUG_OUTPUT("start to call NativeFunction");
        DEBUG_OUTPUT("call NativeFunction"
            + cpp_func->debug_string()
            + "(self=" + (self ? self->debug_string() : "nullptr")
            + ", "+ args_obj->debug_string() + ")"
            );
        model::Object* return_val = cpp_func->func(self, args_list);

        DEBUG_OUTPUT("success to get the result of NativeFunction");

        // 管理返回值引用计数：返回值压栈前必须 make_ref
        if (return_val != nullptr) {
            return_val->make_ref();
        } else {
            // 若返回空，默认压入 Nil（避免栈异常）
            return_val = model::load_nil();
        }

        // 返回值压入操作数栈
        op_stack.push(return_val);

        DEBUG_OUTPUT("ok to call NativeFunction...");
        DEBUG_OUTPUT("NativeFunction return: " + return_val->debug_string());
    } else if (auto* func = dynamic_cast<model::Function*>(func_obj)) {
        // -------------------------- 处理 Function 调用 --------------------------
        DEBUG_OUTPUT("call Function: " + func->name);

        // 校验参数数量
        const size_t required_argc = func->argc;
        const size_t actual_argc = self and self->get_type() != model::Object::ObjectType::OT_Module
            ? args_list->val.size() + 1
            : args_list->val.size();
        if (actual_argc != required_argc) {
            std::cerr << ("CALL: 参数数量不匹配（需" + std::to_string(required_argc) +
                            "个，实际" + std::to_string(actual_argc) + "个）");
            assert(false);
        }

        // 创建新调用帧

        auto new_frame = std::make_shared<CallFrame>(CallFrame{
             func->name,

             func,   // owner
             dep::HashMap<model::Object*>(), // 初始空局部变量表

            0,                               // 程序计数器初始化为0（从第一条指令开始执行）
            call_stack.back()->pc + 1,   // 执行完所有指令后返回的位置（指令池末尾）
            func->code,                 // 关联当前模块的CodeObject

            {}
        });

        // 储存self
        if (self and self->get_type() != model::Object::ObjectType::OT_Module) {
            self->make_ref();
            args_list->val.emplace(args_list->val.begin(), self);
        }

        // 从参数列表中提取参数，存入调用帧 locals
        for (size_t i = 0; i < required_argc; ++i) {
            if (i >= new_frame->code_object->names.size()) {
                assert(false && "CALL: 参数名索引超出范围");
            }

            std::string param_name = new_frame->code_object->names[i];
            model::Object* param_val = args_list->val[i];  // 从列表取参数

            // 校验参数非空
            if (param_val == nullptr) {
                assert(false && ("CALL: 参数" + std::to_string(i) + "为nil（不允许空参数）").c_str());
            }

            // 增加参数引用计数（存入locals需持有引用）
            param_val->make_ref();
            new_frame->locals.insert(param_name, param_val);
        }

        // 压入新调用帧，更新程序计数器
        call_stack.emplace_back(std::move(new_frame));

    // 处理对象魔术方法__call__
    } else {
        try {
            const auto callable = get_attr(func_obj, "__call__");
            DEBUG_OUTPUT("call callable obj");
            handle_call(callable, args_obj, func_obj);
        } catch (NativeFuncError& e) {
            throw NativeFuncError("TypeError", "try to call an uncallable object");
        }
    }
}

void Vm::call_function(model::Object* func_obj, model::Object* args_obj, model::Object* self) {
    size_t old_call_stack_size = call_stack.size();

    handle_call(func_obj, args_obj, self);

    if (old_call_stack_size == call_stack.size()) return;

    while (running and !call_stack.empty() and call_stack.size() > old_call_stack_size) {
        auto& curr_frame = *call_stack.back();
        auto& frame_code = curr_frame.code_object;

        // 检查是否执行到模块代码末尾：执行完毕则出栈
        if (curr_frame.pc >= frame_code->code.size()) {
            call_stack.pop_back();
            continue;
        }

        // 执行当前指令
        const Instruction& curr_inst = frame_code->code[curr_frame.pc];
        try {
            if (curr_inst.opc == Opcode::RET and old_call_stack_size == call_stack.size() - 1) {
                assert(!call_stack.empty());
                call_stack.pop_back();
                return;
            }
            execute_instruction(curr_inst); // 调用VM的指令执行核心方法
        } catch (const NativeFuncError& e) {
            // 原生函数执行错误，抛出异常
            instruction_throw(e.name, e.msg);
            return;
        } catch (const KizStopRunningSignal& e) {
            // 模块执行中触发停止信号，终止执行
            running = false;
            return;
        }

        if (curr_inst.opc != Opcode::JUMP && curr_inst.opc != Opcode::JUMP_IF_FALSE &&
            curr_inst.opc != Opcode::RET && curr_inst.opc != Opcode::JUMP_IF_FINISH_HANDLE_ERROR
            && curr_inst.opc != Opcode::THROW) {
            curr_frame.pc++;
            }

    }
}

void Vm::call_method(model::Object* obj, const std::string& attr_name, model::List* args) {
    assert(obj != nullptr);
    auto parent_it = obj->attrs.find("__parent__");
    std::vector<std::string> magic_methods = {
        "__add__", "__sub__", "__mul__", "__div__", "__pow__", "__pow__",
        "__neg__", "__eq__", "__gt__", "__lt__", "__str__", "__dstr__",
        "__bool__", "__getitem__", "__setitem__", "contains", "__next__", "__hash__"
    };
    const bool attr_is_magic = std::ranges::find(magic_methods, attr_name) != magic_methods.end();
    if (!attr_is_magic or obj == model::based_obj) {
        call_function(get_attr(obj, attr_name), args, obj);
        return;
    }

    if (parent_it) {
        call_function(get_attr(parent_it->value, attr_name), args, obj);
        return;
    }
    throw NativeFuncError("NameError",
        "Undefined method '" + attr_name + "'" + " of " + obj->debug_string()
    );
}

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
    model::Object* func_obj = op_stack.top();
    op_stack.pop();
    func_obj->make_ref();  // 临时持有函数对象，避免中途被释放

    // 弹出栈顶-1元素 : 参数列表
    model::Object* args_obj = op_stack.top();
    op_stack.pop();

    DEBUG_OUTPUT("弹出函数对象: " + func_obj->debug_string());
    DEBUG_OUTPUT("弹出参数列表: " + args_obj->debug_string());
    handle_call(func_obj, args_obj);

}

void Vm::exec_CALL_METHOD(const Instruction& instruction) {
    DEBUG_OUTPUT("exec call method...");

    // 弹出栈顶元素 : 源对象
    auto obj = op_stack.top();
    op_stack.pop();
    obj->make_ref();

    // 弹出栈顶-1元素 : 参数列表
    model::Object* args_obj = op_stack.top();
    op_stack.pop();

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

}

void Vm::exec_RET(const Instruction& instruction) {
    DEBUG_OUTPUT("exec ret...");
    // 兼容顶层调用帧返回
    if (call_stack.size() < 2) {
        if (!op_stack.empty()) {
            op_stack.pop(); // 清理栈顶返回值
        }
        call_stack.pop_back(); // 弹出最后一个帧
        return;
    }

    auto curr_frame = call_stack.back();
    call_stack.pop_back();

    CallFrame* caller_frame = call_stack.back().get();

    model::Object* return_val = new model::Nil();
    return_val->make_ref();
    if (!op_stack.empty()) {
        return_val->del_ref();
        return_val = op_stack.top();
        op_stack.pop();
        return_val->make_ref();
    }

    caller_frame->pc = curr_frame->return_to_pc;
    op_stack.push(return_val);
}
}
