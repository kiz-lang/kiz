#include <cassert>

#include "../models/models.hpp"
#include "vm.hpp"

namespace kiz {

model::Object* Vm::get_attr(const model::Object* obj, const std::string& attr_name) {
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
        "Undefined attribute '" + attr_name + "'" + " of " + obj->debug_string()
    );
}

// -------------------------- 变量操作 --------------------------
void Vm::exec_LOAD_VAR(const Instruction& instruction) {
    DEBUG_OUTPUT("exec load_var...");
    if (call_stack.empty() || instruction.opn_list.empty()) {
        assert(false && "LOAD_VAR: 无调用帧或无变量名索引");
    }

    size_t name_idx = instruction.opn_list[0];
    std::string var_name = call_stack.back()->code_object->names[name_idx];
    
    // 遍历调用栈
    std::shared_ptr<dep::HashMap<model::Object*>::Node> var_it;
    assert(!call_stack.empty());
    for (auto frame_it = call_stack.rbegin(); frame_it != call_stack.rend(); ++frame_it) {
        const CallFrame* curr_frame = (*frame_it).get();

        DEBUG_OUTPUT(var_name);
        var_it = curr_frame->locals.find(var_name);
        if (var_it) break;
    }

    if (var_it == nullptr) {
        DEBUG_OUTPUT("try to find in builtins (getting var name)");
        var_name = call_stack.back().get()-> code_object->names[name_idx];
        DEBUG_OUTPUT("current builtins: " + builtins.to_string());
        DEBUG_OUTPUT("var_name="+var_name);
        if (auto builtin_it = builtins.find(var_name)) {
            model::Object* builtin_val = builtin_it->value;
            op_stack.push(builtin_val);
            return;
        }
        if (auto owner_module_it = call_stack.back()->owner->attrs.find("__owner_module__")) {
            auto owner_module = dynamic_cast<model::Module*>(owner_module_it->value);
            assert(owner_module != nullptr);

            auto mod_var_it = owner_module->attrs.find(var_name);
            if (mod_var_it) {
                model::Object* var_val = mod_var_it->value;
                var_val->make_ref();
                op_stack.push(var_val);
                return;
            }
        }
        else {
            DEBUG_OUTPUT("var_name = " << var_name << ", not found, throwing error with instruction_throw");
            instruction_throw("NameError", "Undefined variable '"+var_name+"'");
        }
    }

    model::Object* var_val = var_it->value;
    DEBUG_OUTPUT("load var: " + var_name + " = " + var_val->debug_string());
    var_val->make_ref();
    op_stack.push(var_val);
    DEBUG_OUTPUT("ok to exec load_var...");
}

void Vm::exec_LOAD_CONST(const Instruction& instruction) {
    DEBUG_OUTPUT("exec load_const...");
    if (instruction.opn_list.empty()) {
        assert(false && "LOAD_CONST: 无常量索引");
    }
    size_t const_idx = instruction.opn_list[0];
    CallFrame* frame = call_stack.back().get();
    if (const_idx >= frame->code_object->consts.size()) {
        assert(false && "LOAD_CONST: 常量索引超出范围");
    }
    model::Object* const_val = frame->code_object->consts[const_idx];
    DEBUG_OUTPUT("ok to get load const [" + std::to_string(const_idx) + "]: "+ const_val->debug_string());
    const_val->make_ref();
    op_stack.push(const_val);
}

void Vm::exec_SET_GLOBAL(const Instruction& instruction) {
    DEBUG_OUTPUT("exec set_global...");
    if (call_stack.empty() || op_stack.empty() || instruction.opn_list.empty()) {
        assert(false && "SET_GLOBAL: 无调用帧/栈空/无变量名索引");
    }
    CallFrame* global_frame = call_stack.front().get();
    size_t name_idx = instruction.opn_list[0];
    if (name_idx >= global_frame->code_object->names.size()) {
        assert(false && "SET_GLOBAL: 变量名索引超出范围");
    }
    std::string var_name = call_stack.back()->code_object->names[name_idx];

    model::Object* var_val = fetch_one_from_stack_top();
    var_val = model::copy_or_ref(var_val);

    auto var_it = global_frame->locals.find(var_name);

    global_frame->locals.insert(var_name, var_val);
}

void Vm::exec_SET_LOCAL(const Instruction& instruction) {
    DEBUG_OUTPUT("exec set_local...");
    if (call_stack.empty() || op_stack.empty() || instruction.opn_list.empty()) {
        assert(false && "SET_LOCAL: 无调用帧/栈空/无变量名索引");
    }
    CallFrame* curr_frame = call_stack.back().get();
    size_t name_idx = instruction.opn_list[0];
    if (name_idx >= curr_frame->code_object->names.size()) {
        assert(false && "SET_LOCAL: 变量名索引超出范围");
    }
    const std::string var_name = curr_frame->code_object->names[name_idx];
    DEBUG_OUTPUT("ok to get var name: " + var_name);

    model::Object* var_val = fetch_one_from_stack_top();
    var_val = model::copy_or_ref(var_val);
    DEBUG_OUTPUT("var val: " + var_val->debug_string());

    auto var_it = curr_frame->locals.find(var_name);

    curr_frame->locals.insert(var_name, var_val);
    DEBUG_OUTPUT("ok to set_local...");
    DEBUG_OUTPUT("current local at [" + std::to_string(call_stack.size()) + "] " +  curr_frame->locals.to_string());
}

void Vm::exec_SET_NONLOCAL(const Instruction& instruction) {
    DEBUG_OUTPUT("exec set_nonlocal...");
    if (call_stack.size() < 2 || op_stack.empty() || instruction.opn_list.empty()) {
        assert(false && "SET_NONLOCAL: 调用帧不足/栈空/无变量名索引");
    }
    size_t name_idx = instruction.opn_list[0];
    std::string var_name = call_stack.back()->code_object->names[name_idx];
    CallFrame* target_frame = nullptr;

    auto frame_it = call_stack.rbegin();
    ++frame_it;
    for (; frame_it != call_stack.rend(); ++frame_it) {
        CallFrame* frame = frame_it->get();
        if (name_idx >= frame->code_object->names.size()) continue;
        if (frame->locals.find(var_name)) {
            target_frame = frame;
            break;
        }
    }

    if (!target_frame) {
        instruction_throw("NameError", "Undefined variable '"+var_name+"'");
        assert(false);
    }

    model::Object* var_val = fetch_one_from_stack_top();
    var_val = model::copy_or_ref(var_val);

    target_frame->locals.insert(var_name, var_val);
}

// -------------------------- 属性访问 --------------------------
void Vm::exec_GET_ATTR(const Instruction& instruction) {
    DEBUG_OUTPUT("exec get_attr...");
    if (op_stack.empty() || instruction.opn_list.empty()) {
        assert(false && "GET_ATTR: 操作数栈为空或无属性名索引");
    }
    model::Object* obj = op_stack.top();
    op_stack.pop();
    size_t name_idx = instruction.opn_list[0];
    CallFrame* curr_frame = call_stack.back().get();

    if (name_idx >= curr_frame->code_object->names.size()) {
        assert(false && "GET_ATTR: 属性名索引超出范围");
    }
    std::string attr_name = curr_frame->code_object->names[name_idx];
    DEBUG_OUTPUT("attr name: " + attr_name);
    DEBUG_OUTPUT("obj: " + obj->debug_string());

    model::Object* attr_val = get_attr(obj, attr_name);
    DEBUG_OUTPUT("attr val: " + attr_val->debug_string());
    op_stack.push(attr_val);
}

void Vm::exec_SET_ATTR(const Instruction& instruction) {
    DEBUG_OUTPUT("exec set_attr...");
    if (op_stack.size() < 2 || instruction.opn_list.empty()) {
        assert(false && "SET_ATTR: 操作数栈元素不足或无属性名索引");
    }
    model::Object* attr_val = fetch_one_from_stack_top();
    attr_val = model::copy_or_ref(attr_val);
    model::Object* obj = fetch_one_from_stack_top();
    size_t name_idx = instruction.opn_list[0];
    CallFrame* curr_frame = call_stack.back().get();

    if (name_idx >= curr_frame->code_object->names.size()) {
        assert(false && "SET_ATTR: 属性名索引超出范围");
    }
    std::string attr_name = curr_frame->code_object->names[name_idx];

    auto attr_it = obj->attrs.find(attr_name);

    attr_val->make_ref();
    obj->attrs.insert(attr_name, attr_val);
}

void Vm::exec_GET_ITEM(const Instruction& instruction) {
    model::Object* obj = fetch_one_from_stack_top();

    auto args_list = dynamic_cast<model::List*>(fetch_one_from_stack_top());
    assert(args_list != nullptr);

    handle_call(get_attr(obj, "__getitem__"), args_list, obj);
}

void Vm::exec_SET_ITEM(const Instruction& instruction) {
    model::Object* value = fetch_one_from_stack_top();
    model::Object* arg = fetch_one_from_stack_top();
    model::Object* obj = fetch_one_from_stack_top();

    // 获取对象自身的 __setitem__
    model::Object* setitem_method = get_attr(obj, "__setitem__");

    handle_call(setitem_method, new model::List({arg, value}), obj);
}

}
