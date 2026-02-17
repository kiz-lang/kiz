#include <utility>

#include "vm.hpp"
#include "../models/models.hpp"
#include "opcode/opcode.hpp"
#include <unordered_set>

namespace kiz {

bool Vm::is_true(model::Object* obj) {
    if (const auto bool_obj = dynamic_cast<const model::Bool*>(obj)) {
        return bool_obj->val==true;
    }
    if (dynamic_cast<const model::Nil*>(obj)) {
        return false;
    }

    call_method(obj, "__bool__", {});
    auto result = simple_get_and_pop_stack_top();
    bool ret = is_true(result);

    return ret;
}

model::Object* Vm::get_attr(model::Object* obj, const std::string& attr_name) {
    assert(obj != nullptr);
    const auto attr_it = obj->attrs.find(attr_name);
    auto parent_it = obj->attrs.find("__parent__");
    if (attr_it) {
        return attr_it->value;
    }

    if (parent_it) {
        return get_attr(parent_it->value, attr_name);
    }

    throw NativeFuncError("NameError",
        "Undefined attribute '" + attr_name
    );
}

model::Object* Vm::get_attr_current(model::Object* obj, const std::string& attr) {
    const auto attr_it = obj->attrs.find(attr);
    if (attr_it) {
        return attr_it->value;
    }
    throw NativeFuncError("NameError",
        "Undefined attribute '" + attr + "'" + " of current attributes table"
    );
}

void Vm::handle_call(model::Object* func_obj, model::Object* args_obj, model::Object* self){
    assert(func_obj != nullptr);
    assert(args_obj != nullptr);
    auto args_list = dynamic_cast<model::List*>(args_obj);

    assert(args_list && "CALL: 栈顶-1元素非List类型（参数必须封装为列表）");
    DEBUG_OUTPUT("start to call function");

    // 分类型处理函数调用（Function / NativeFunction）
    if (const auto cpp_func = dynamic_cast<model::NativeFunction*>(func_obj)) {
        // -------------------------- 处理 NativeFunction 调用 --------------------------
        model::Object* return_val = cpp_func->func(self, args_list);

        // 管理返回值引用计数：返回值压栈前必须 make_ref
        if (!return_val){
            // 若返回空，默认压入 Nil（避免栈异常）
            return_val = model::unique_nil;
        }

        // 返回值压入操作数栈
        push_to_stack(return_val);
    } else if (auto func = dynamic_cast<model::Function*>(func_obj)) {
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
        func->make_ref();
        func->code->make_ref();
        auto new_frame = new CallFrame{
            .name = func->name,

            .owner = func,

            .pc = 0,
            .return_to_pc = call_stack.back()->pc + 1,
            .last_bp = call_stack.back()->bp,
            .bp = op_stack.size(),
            .code_object = func->code,

            .iters{},

            .curr_error = nullptr,
            .exec_ensure_stmt = false
        };
        op_stack.resize(op_stack.size() + func->code->locals_count);

        // 储存self
        if (self and self->get_type() != model::Object::ObjectType::Module) {
            self->make_ref();
            args_list->val.emplace(args_list->val.begin(), self);
        }

        if (func->has_rest_params) {
            for (size_t i = 0; i < required_argc; ++i) {
                std::string param_name = new_frame->code_object->var_names[i];
                model::Object* param_val;
                if (i == required_argc - 1 ) {
                    args_list->val.assign(args_list->val.begin() + i, args_list->val.end());
                    param_val = args_list;
                }
                else {
                    param_val = args_list->val[i];  // 从列表取参数
                }

                assert(param_val);
                param_val->make_ref();
                op_stack[new_frame->bp + i] = param_val;
            }

            call_stack.emplace_back(new_frame);
            return;
        }

        // 从参数列表中提取参数，存入locals
        for (size_t i = 0; i < required_argc; ++i) {
            std::string param_name = new_frame->code_object->var_names[i];
            model::Object* param_val = args_list->val[i];  // 从列表取参数

            // 校验参数非空
            assert(param_val != nullptr);

            // 增加参数引用计数（存入locals需持有引用）
            param_val->make_ref();
            op_stack[new_frame->bp + i] = param_val;
        }

        // 压入新调用帧，更新程序计数器
        call_stack.emplace_back(new_frame);

    // 处理对象魔术方法__call__
    } else {
        model::Object* callable;
        try {
            callable = get_attr(func_obj, "__call__");
        } catch (NativeFuncError& e) {
            throw NativeFuncError("TypeError", "try to call an uncallable object");
        }
        assert(callable);
        handle_call(callable, args_obj, func_obj);
    }
}

void Vm::call_function(model::Object* func_obj, std::vector<model::Object*> args, model::Object* self) {
    size_t old_call_stack_size = call_stack.size();

    auto temp_args = new model::List(std::move(args));
    temp_args->make_ref();
    handle_call(func_obj, temp_args, self);
    temp_args->del_ref();

    if (old_call_stack_size == call_stack.size()) return;

    while (running and !call_stack.empty() and call_stack.size() > old_call_stack_size) {
        auto curr_frame = call_stack.back();
        auto& frame_code = curr_frame->code_object;

        // 检查是否执行到模块代码末尾：执行完毕则出栈
        if (curr_frame->pc >= frame_code->code.size()) {
            call_stack.pop_back();
            continue;
        }

        // 执行当前指令
        const Instruction& curr_inst = frame_code->code[curr_frame->pc];
        try {
            if (curr_inst.opc == Opcode::RET and old_call_stack_size == call_stack.size() - 1) {
                assert(!call_stack.empty());
                call_stack.pop_back();
                return;
            }
            execute_unit(curr_inst); // 调用VM的指令执行核心方法
        } catch (const NativeFuncError& e) {
            // 原生函数执行错误，抛出异常
            forward_to_handle_throw(e.name, e.msg);
            continue;
        }

        ADVANCE_PC

    }
}

void Vm::call_method(model::Object* obj, const std::string& attr_name, std::vector<model::Object*> args) {
    assert(obj != nullptr);
    auto parent_it = obj->attrs.find("__parent__");
    static const std::unordered_set<std::string_view> magic_methods = {
    std::string_view("__add__"), std::string_view("__sub__"),
    std::string_view("__mul__"), std::string_view("__div__"),
    std::string_view("__pow__"), std::string_view("__mod__"),
    std::string_view("__neg__"), std::string_view("__eq__"),
    std::string_view("__gt__"), std::string_view("__lt__"),
    std::string_view("__str__"), std::string_view("__dstr__"),
    std::string_view("__bool__"), std::string_view("__getitem__"),
    std::string_view("__setitem__"), std::string_view("contains"),
    std::string_view("__next__"), std::string_view("__hash__")
    };

    const bool is_magic = magic_methods.contains(std::string_view(attr_name));
    if (!is_magic) {
        call_function(get_attr(obj, attr_name), args, obj);
        return;
    }

    if (parent_it) {
        call_function(get_attr(parent_it->value, attr_name), args, obj);
        return;
    }
    throw NativeFuncError("NameError",
        "Undefined method '" + attr_name + "'"
    );
}

std::string Vm::obj_to_str(model::Object* for_cast_obj) {
    DEBUG_OUTPUT("obj to str");
    try {
        call_method(for_cast_obj, "__str__", {});
    } catch (NativeFuncError& e) {
        call_method(for_cast_obj, "__dstr__", {});
    }
    auto res = simple_get_and_pop_stack_top();
    std::string val = model::cast_to_str(res)->val;
    return val;
}


std::string Vm::obj_to_debug_str(model::Object* for_cast_obj) {
    DEBUG_OUTPUT("obj to debug str");
    try {
        call_method(for_cast_obj, "__dstr__", {});
    } catch (NativeFuncError& e) {
        call_method(for_cast_obj, "__str__", {});
    }
    auto res = simple_get_and_pop_stack_top();
    std::string val = model::cast_to_str(res) ->val;
    return val;
}
}
