#include "vm.hpp"
#include "../../libs/builtins/include/builtin_functions.hpp"
#include "../opcode/opcode.hpp"

///| 核心执行单元
namespace kiz {
void Vm::execute_unit(const Instruction& instruction) {
    switch (instruction.opc) {
    case Opcode::OP_ADD: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();
        // call_method(a, "__add__", {b});
        // 释放使用后的操作数
        a->del_ref();
        b->del_ref();
    }

    case Opcode::OP_SUB: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();
        // call_method(a, "__sub__", {b});
        // 释放使用后的操作数
        a->del_ref();
        b->del_ref();
    }

    case Opcode::OP_MUL: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();
        // call_method(a, "__mul__", {b});
        // 释放使用后的操作数
        a->del_ref();
        b->del_ref();
    }

    case Opcode::OP_DIV: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();
        // call_method(a, "__div__", {b});
        // 释放使用后的操作数
        a->del_ref();
        b->del_ref();
    }

    case Opcode::OP_MOD: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();
        // call_method(a, "__mod__", {b});
        // 释放使用后的操作数
        a->del_ref();
        b->del_ref();
    }

    case Opcode::OP_POW: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();
        // call_method(a, "__pow__", {b});
        // 释放使用后的操作数
        a->del_ref();
        b->del_ref();
    }

    case Opcode::OP_NEG: {
        auto a = fetch_one_from_stack_top();
        // call_method(a, "__neg__", {});
        // 释放使用后的操作数
        a->del_ref();
    }

    case Opcode::OP_EQ: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();

        //call_method(a, "__eq__", {b});
        a->del_ref();
        b->del_ref();
    }

    case Opcode::OP_GT: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();

        //call_method(a, "__gt__", {b});
        a->del_ref();
        b->del_ref();
    }

    case Opcode::OP_LT: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();

        //call_method(a, "__lt__", {b});
        a->del_ref();
        b->del_ref();
    }

    case Opcode::OP_GE: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();

        //call_method(a, "__eq__", {b});
        //call_method(a, "__gt__", {b});

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

    case Opcode::OP_LE: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();

        // 调用__eq__方法
        //call_method(a, "__eq__", {b});

        // 调用__lt__方法
        //call_method(a, "__lt__", {b});

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

    case Opcode::OP_NE: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();

        // 调用__eq__方法
        //call_method(a, "__eq__", {b});

        // 获取比较结果
        auto eq_result = fetch_one_from_stack_top();

        // 压入取反结果
        push_to_stack(model::load_bool(
            is_true(eq_result)
        ));

        // 释放所有使用后的对象
        a->del_ref();
        b->del_ref();
        eq_result->del_ref();
    }

    case Opcode::OP_NOT: {
        auto a = fetch_one_from_stack_top();
        bool result = !is_true(a);
        push_to_stack(model::load_bool(result));

        // 释放使用后的操作数
        a->del_ref();
    }

    case Opcode::OP_IS: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();
        // 压入比较结果
        push_to_stack(model::load_bool(a == b));
        // 释放使用后的操作数
        a->del_ref();
        b->del_ref();
    }

    case Opcode::OP_IN: {
        auto for_check = fetch_one_from_stack_top();
        auto item = fetch_one_from_stack_top();

        // 调用contains方法，参数为item
        // call_method(for_check, "contains", {item});

        // 释放使用后的对象
        for_check->del_ref();
        item->del_ref();
    }

    case Opcode::MAKE_LIST: {
        // make_list(len)
    }

    case Opcode::MAKE_DICT: {
        // make_dict(len)
    }


    case Opcode::CALL: {
        model::Object* func_obj = fetch_one_from_stack_top();
        func_obj->make_ref();
        // 弹出栈顶-1元素 : 参数列表
        model::Object* args_obj = fetch_one_from_stack_top();
        handle_call(func_obj, args_obj, nullptr);

        func_obj->del_ref();
    }

    case Opcode::RET: {
        // handle_return()
    }

    case Opcode::CALL_METHOD: {
        auto obj = fetch_one_from_stack_top();
        obj->make_ref();

        // 弹出栈顶-1元素 : 参数列表
        model::Object* args_obj = fetch_one_from_stack_top();

        // std::string attr_name = get_name_by_idx(name_idx);

        //auto func_obj = get_attr(obj, attr_name);
        //func_obj->make_ref();

        //handle_call(func_obj, args_obj, obj);

        //func_obj->del_ref();
        obj->del_ref();
    }

    case Opcode::GET_ATTR: {
        model::Object* obj = fetch_one_from_stack_top();
        // std::string attr_name = get_name_by_idx(name_idx);

        //model::Object* attr_val = get_attr(obj, attr_name);
        //push_to_stack(attr_val);
        obj->del_ref();
    }

    case Opcode::SET_ATTR: {
        model::Object* attr_val = fetch_one_from_stack_top();
        attr_val = model::copy_or_ref(attr_val);

        model::Object* obj = fetch_one_from_stack_top();
        // std::string attr_name = get_name_by_idx(name_idx);

        // 检查原有同名属性，存在则释放引用
        //auto attr_it = obj->attrs.find(attr_name);
        // if (attr_it) {
        //     // ==| IMPORTANT |==
        //     attr_it->value->del_ref();
        // }

        // obj->attrs.insert(attr_name, attr_val);
        attr_val->del_ref();
        obj->del_ref();
    }

    case Opcode::GET_ITEM: {
        model::Object* obj = fetch_one_from_stack_top();
        auto args_list = model::cast_to_list(fetch_one_from_stack_top());

        //call_method(obj, "__getitem__", args_list->val);
        args_list->del_ref();
        obj->del_ref();
    }

    case Opcode::SET_ITEM: {
        model::Object* value = fetch_one_from_stack_top();
        model::Object* arg = fetch_one_from_stack_top();
        model::Object* obj = fetch_one_from_stack_top();

        // 获取对象自身的 __setitem__
        //call_method(obj, "__setitem__", {arg, value});

        // 释放所有临时对象
        value->del_ref();
        arg->del_ref();
        obj->del_ref();
    }

    case Opcode::LOAD_VAR: {
        // load_var
    }

    case Opcode::LOAD_CONST: {
        size_t const_idx = instruction.opn_list[0];
        model::Object* const_val = const_pool[const_idx];
        push_to_stack(const_val);
    }

    case Opcode::SET_GLOBAL: {
        CallFrame* global_frame = call_stack.front().get();
        // std::string var_name = get_name_by_idx(name_idx);
        model::Object* var_val = fetch_one_from_stack_top();
        const auto new_val = model::copy_or_ref(var_val); // 得到新对象
        //
        // // 释放原有同名变量
        // auto var_it = global_frame->locals.find(var_name);
        // if (var_it) {
        //     var_it->value->del_ref();
        // }
        //
        // global_frame->locals.insert(var_name, new_val);
        var_val->del_ref();
    }

    case Opcode::SET_LOCAL: {
        CallFrame* curr_frame = call_stack.back().get();
        // std::string var_name = get_name_by_idx(name_idx);

        model::Object* var_val = fetch_one_from_stack_top();
        auto new_val = model::copy_or_ref(var_val);

        // 检查原有同名变量，存在则释放引用
        //auto var_it = curr_frame->locals.find(var_name);
        // if (var_it) {
        //     var_it->value->del_ref();
        // }

        // 插入new_val，而非原var_val
        //curr_frame->locals.insert(var_name, new_val);
        // ==| IMPORTANT |==
        var_val->del_ref();
    }

    case Opcode::SET_NONLOCAL: {
        // set_nonlocal();
    }


    case Opcode::ENTER_TRY: {
        size_t catch_start = instruction.opn_list[0];
        size_t finally_start = instruction.opn_list[1];
        call_stack.back()->try_blocks.emplace_back(false, catch_start, finally_start);
    }

    case Opcode::JUMP_IF_FINISH_HANDLE_ERROR: {
        bool finish_handle_error = call_stack.back()->try_blocks.back().handle_error;
        // 弹出TryFrame
        call_stack.back()->try_blocks.pop_back();
        if (finish_handle_error) {
            call_stack.back()->pc = instruction.opn_list[0];
        } else {
            call_stack.back()->pc ++;
        }
    }

    case Opcode::MARK_HANDLE_ERROR: {
        call_stack.back()->try_blocks.back().handle_error = true;
    }

    case Opcode::THROW: {
        const auto top = fetch_one_from_stack_top();
        top->make_ref();

        // 替换前释放旧错误对象
        if (curr_error) {
            curr_error->del_ref();
        }
        curr_error = top;
        handle_throw();
    }

    case Opcode::LOAD_ERROR: {
        assert(curr_error != nullptr);
        push_to_stack(curr_error);
    }

    case Opcode::JUMP: {
        size_t target_pc = instruction.opn_list[0];
        call_stack.back()->pc = target_pc;
    }

    case Opcode::JUMP_IF_FALSE: {
        model::Object* cond = fetch_one_from_stack_top();
        if (! is_true(cond)) {
            // 跳转逻辑
            call_stack.back()->pc = instruction.opn_list[0];
        } else {
            call_stack.back()->pc++;
        }
        cond->del_ref();
    }

    case Opcode::IS_CHILD: {
        auto b = fetch_one_from_stack_top();
        auto a = fetch_one_from_stack_top();
        push_to_stack(builtin::check_based_object(a, b));
        // 释放使用后的a/b对象，计数对称
        a->del_ref();
        b->del_ref();
    }

    case Opcode::CREATE_OBJECT: {
        auto obj = new model::Object();
        obj->make_ref(); // 创建即计数
        obj->attrs_insert("__parent__", model::based_obj);
        push_to_stack(obj); // push自动make_ref()，计数变为3，栈释放后回到2
    }

    case Opcode::IMPORT: {
        //std::string module_path = get_name_by_idx(instruction.opn_list[0]);
        //handle_import(module_path);
    }

    case Opcode::CACHE_ITER: {
        auto iter = op_stack.top();
        iter->make_ref();

        call_stack.back().get()->iters.push_back(iter);
    }

    case Opcode::GET_ITER: {
        assert(!call_stack.empty());
        assert(!call_stack.back().get()->iters.empty());

        push_to_stack(
            call_stack.back().get()->iters.back()
        );
    }

    case Opcode::POP_ITER: {
        assert(!call_stack.empty());
        auto curr_frame = call_stack.back().get();
        assert(!curr_frame->iters.empty());

        auto iter_obj = curr_frame->iters.back();
        iter_obj->del_ref();
        curr_frame->iters.pop_back();
    }

    case Opcode::JUMP_IF_FINISH_ITER: {
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

    case Opcode::COPY_TOP: {
        auto obj = fetch_one_from_stack_top();
        push_to_stack(obj);
        push_to_stack(obj);
    }

    case Opcode::STOP: {
        running = false;
    }

    default: throw NativeFuncError("FutureError", "execute_instruction meet unknown opcode");
    }

}
}
