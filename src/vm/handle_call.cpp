#include "vm.hpp"
#include "../models/models.hpp"
#include "opcode/opcode.hpp"

namespace kiz {

bool Vm::is_true(model::Object* obj) {
    if (const auto bool_obj = dynamic_cast<const model::Bool*>(obj)) {
        return bool_obj->val==true;
    }
    if (dynamic_cast<const model::Nil*>(obj)) {
        return false;
    }

    // 修复：先创建列表，管理所有权
    auto temp_list = model::create_list({});
    model::List* args_list = model::cast_to_list(temp_list);
    temp_list->del_ref(); // 移交所有权给 args_list，计数平衡

    call_method(obj, "__bool__", args_list);
    auto result = fetch_one_from_stack_top();
    bool ret = is_true(result);

    args_list->del_ref();
    result->del_ref();
    return ret;
}


void Vm::handle_call(model::Object* func_obj, model::Object* args_obj, model::Object* self){
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
        if (!return_val){
            // 若返回空，默认压入 Nil（避免栈异常）
            return_val = model::unique_nil;
        }

        // 返回值压入操作数栈
        push_to_stack(return_val);

        DEBUG_OUTPUT("ok to call NativeFunction...");
        DEBUG_OUTPUT("NativeFunction return: " + return_val->debug_string());
    } else if (auto* func = dynamic_cast<model::Function*>(func_obj)) {
        // -------------------------- 处理 Function 调用 --------------------------
        DEBUG_OUTPUT("call Function: " + func->name);

        // 校验参数数量
        const size_t required_argc = func->argc;
        size_t actual_argc;
        if (self and self->get_type() != model::Object::ObjectType::Module) {
            actual_argc = args_list->val.size() + 1;
        } else {
            actual_argc = args_list->val.size();
        }
        if (actual_argc != required_argc and !func->has_rest_params)
            throw NativeFuncError("ArgCountError", std::format(
                "expect {} arguments but got {} arguments", required_argc, actual_argc
            ));

        // 创建新调用帧

        auto new_frame = std::make_shared<CallFrame>(
             func->name,

             func,   // owner
             dep::HashMap<model::Object*>(), // 初始空局部变量表

            0,                               // 程序计数器初始化为0（从第一条指令开始执行）
            call_stack.back()->pc + 1,       // 执行完所有指令后返回的位置（指令池末尾）
            func->code,                      // 关联当前模块的CodeObject

            std::vector<TryFrame>{},

            std::vector<model::Object*>{}
        );

        // 储存self
        if (self and self->get_type() != model::Object::ObjectType::Module) {
            self->make_ref();
            args_list->val.emplace(args_list->val.begin(), self);
        }

        if (func->has_rest_params) {
            for (size_t i = 0; i < required_argc; ++i) {
                std::string param_name = new_frame->code_object->names[i];
                model::Object* param_val;
                if (i == required_argc - 1 ) {
                    args_list->val.assign(args_list->val.begin() + i, args_list->val.end());
                    param_val = args_list;
                }
                else {
                    param_val = args_list->val[i];  // 从列表取参数
                }

                assert(param_val != nullptr && ("CALL: 参数" + std::to_string(i) + "为nil（不允许空参数）").c_str());

                new_frame->locals.insert(param_name, param_val);
            }

            call_stack.emplace_back(std::move(new_frame));
            return;
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
            && curr_inst.opc != Opcode::THROW && curr_inst.opc != Opcode::JUMP_IF_FINISH_ITER) {
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

std::string Vm::obj_to_str(model::Object* for_cast_obj) {
    DEBUG_OUTPUT("obj to str");
    try {
        call_method(for_cast_obj, "__str__", model::create_list({}));
    } catch (NativeFuncError& e) {
        call_method(for_cast_obj, "__dstr__", model::create_list({}));
    }
    auto res = fetch_one_from_stack_top();
    std::string val = model::cast_to_str(res)->val;

    return val;
}


std::string Vm::obj_to_debug_str(model::Object* for_cast_obj) {
    DEBUG_OUTPUT("obj to debug str");
    try {
        call_method(for_cast_obj, "__dstr__", model::create_list({}));
    } catch (NativeFuncError& e) {
        call_method(for_cast_obj, "__str__", model::create_list({}));
    }
    auto res = fetch_one_from_stack_top();
    std::string val = model::cast_to_str(res)->val;
    return val;
}
}
