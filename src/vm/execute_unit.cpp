#include "vm.hpp"
#include "../../libs/builtins/include/builtin_functions.hpp"
#include "../opcode/opcode.hpp"

///| 核心执行单元
namespace kiz {
void Vm::execute_unit(const Instruction& instruction) {
    switch (instruction.opc) {
    case Opcode::OP_ADD: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();
        call_method(a, "__add__", {b});
        b->del_ref();
        a->del_ref();
        break;
    }

    case Opcode::OP_SUB: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();
        call_method(a, "__sub__", {b});
        b->del_ref();
        a->del_ref();
        break;
    }

    case Opcode::OP_MUL: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();
        call_method(a, "__mul__", {b});
        b->del_ref();
        a->del_ref();
        break;
    }

    case Opcode::OP_DIV: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();
        call_method(a, "__div__", {b});
        b->del_ref();
        a->del_ref();
        break;
    }

    case Opcode::OP_MOD: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();
        call_method(a, "__mod__", {b});
        b->del_ref();
        a->del_ref();
        break;
    }

    case Opcode::OP_POW: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();
        call_method(a, "__pow__", {b});
        b->del_ref();
        a->del_ref();
        break;
    }

    case Opcode::OP_NEG: {
        auto a = get_and_pop_stack_top();
        call_method(a, "__neg__", {});
        a->del_ref();
        break;
    }

    case Opcode::OP_EQ: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();

        call_method(a, "__eq__", {b});
        b->del_ref();
        a->del_ref();
        break;
    }

    case Opcode::OP_GT: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();

        call_method(a, "__gt__", {b});
        b->del_ref();
        a->del_ref();
        break;
    }

    case Opcode::OP_LT: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();

        call_method(a, "__lt__", {b});
        b->del_ref();
        a->del_ref();
        break;
    }

    case Opcode::OP_GE: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();

        call_method(a, "__eq__", {b});
        call_method(a, "__gt__", {b});

        // 统一使用封装的栈操作获取结果
        auto gt_result = get_and_pop_stack_top();
        auto eq_result = get_and_pop_stack_top();

        // 压入最终结果
        if (is_true(gt_result) or is_true(eq_result)) {
            push_to_stack(model::load_true());
        } else {
            push_to_stack(model::load_false());
        }
        b->del_ref();
        a->del_ref();
        eq_result->del_ref();
        gt_result->del_ref();
        break;
    }

    case Opcode::OP_LE: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();

        // 调用__eq__方法
        call_method(a, "__eq__", {b});

        // 调用__lt__方法
        call_method(a, "__lt__", {b});

        // 获取结果
        auto lt_result = get_and_pop_stack_top();
        auto eq_result = get_and_pop_stack_top();

        // 压入最终结果
        if (is_true(lt_result) or is_true(eq_result)) {
            push_to_stack(model::load_true());
        } else {
            push_to_stack(model::load_false());
        }
        b->del_ref();
        a->del_ref();
        eq_result->del_ref();
        lt_result->del_ref();
        break;
    }

    case Opcode::OP_NE: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();


        // 调用__eq__方法
        call_method(a, "__eq__", {b});

        // 获取比较结果
        auto eq_result = get_and_pop_stack_top();

        // 压入取反结果
        push_to_stack(model::load_bool(
            is_true(eq_result)
        ));

        b->del_ref();
        a->del_ref();
        eq_result->del_ref();
        break;
    }

    case Opcode::OP_NOT: {
        auto a = get_and_pop_stack_top();
        bool result = !is_true(a);
        a->del_ref();
        push_to_stack(model::load_bool(result));
        break;
    }

    case Opcode::OP_IS: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();
        // 压入比较结果
        b->del_ref();
        a->del_ref();
        push_to_stack(model::load_bool(a == b));
        break;
    }

    case Opcode::OP_IN: {
        auto for_check = get_and_pop_stack_top();
        auto item = get_and_pop_stack_top();

        // 调用contains方法，参数为item
        call_method(for_check, "contains", {item});

        for_check->del_ref();
        item->del_ref();
        break;
    }

    case Opcode::MAKE_LIST: {
        make_list(instruction.opn_list[0]);
        break;
    }

    case Opcode::MAKE_DICT: {
        make_dict(instruction.opn_list[0]);
        break;
    }

    case Opcode::CREATE_CLOSURE: {
        auto func_obj = dynamic_cast<model::Function*>(op_stack.back());

        auto& upvalues = func_obj->code->upvalues;
        std::vector<model::Object*> free_vars {};

        for (const auto& [distance_from_curr, idx] : upvalues) {
            auto frame = call_stack[ call_stack.size() - distance_from_curr];
            size_t loc_based = frame->bp;

            auto var = op_stack[loc_based + idx];
            var->make_ref();
            free_vars.push_back( var );
        }

        func_obj->free_vars = free_vars;
        break;
    }


    case Opcode::CALL: {
        model::Object* func_obj = get_and_pop_stack_top();
        // 弹出栈顶-1元素 : 参数列表
        model::Object* args_obj = get_and_pop_stack_top();
        handle_call(func_obj, args_obj, nullptr);
        func_obj->del_ref();
        args_obj->del_ref();
        break;
    }

    case Opcode::RET: {
        auto frame = call_stack.back();
        call_stack.pop_back();
        call_stack.back()->bp = frame->last_bp;
        call_stack.back()->pc = frame->return_to_pc;

        auto return_val = get_and_pop_stack_top();
        assert(return_val);

        while (frame->bp < op_stack.size()) {
            op_stack.back()->del_ref();
            op_stack.pop_back();
        }

        push_to_stack(return_val);

        frame->owner->del_ref();
        frame->code_object->del_ref();
        for (auto it: frame->iters) {
            if (it) it->del_ref();
        }

        delete frame;
        break;
    }

    case Opcode::CALL_METHOD: {
        auto obj = get_and_pop_stack_top();

        // 弹出栈顶-1元素 : 参数列表
        model::Object* args_obj = get_and_pop_stack_top();

        std::string attr_name = get_attr_name_by_idx(instruction.opn_list[0]);

        auto func_obj = get_attr(obj, attr_name);

        func_obj->make_ref();
        handle_call(func_obj, args_obj, obj);

        func_obj->del_ref();
        obj->del_ref();
        args_obj->del_ref();
        break;
    }

    case Opcode::GET_ATTR: {
        model::Object* obj = get_and_pop_stack_top();
         std::string attr_name = get_attr_name_by_idx(instruction.opn_list[0]);

        model::Object* attr_val = get_attr(obj, attr_name);
        push_to_stack(attr_val);
        obj->del_ref();
        break;
    }

    case Opcode::SET_ATTR: {
        model::Object* attr_val = get_and_pop_stack_top();
        model::Object* obj = get_and_pop_stack_top();
        std::string attr_name = get_attr_name_by_idx(instruction.opn_list[0]);

        auto new_val = model::copy_if_mutable(attr_val);
        auto old_it = obj->attrs.find(attr_name);   // 获取旧值（若有）
        obj->attrs_insert(attr_name, new_val);      // 插入新值，内部 make_ref
        if (old_it) old_it->value->del_ref();       // 释放旧值

        attr_val->del_ref();
        obj->del_ref();
        break;
    }

    case Opcode::GET_ITEM: {
        model::Object* obj = get_and_pop_stack_top();
        auto args_list = model::cast_to_list(get_and_pop_stack_top());

        call_method(obj, "__getitem__", args_list->val);
        args_list->del_ref();
        obj->del_ref();
        break;
    }

    case Opcode::SET_ITEM: {
        model::Object* value = get_and_pop_stack_top();
        model::Object* arg = get_and_pop_stack_top();
        model::Object* obj = get_and_pop_stack_top();

        // 获取对象自身的 __setitem__
        call_method(obj, "__setitem__", {arg, value});
        obj->del_ref();
        value->del_ref();
        arg->del_ref();
        break;
    }

    case Opcode::LOAD_VAR: {
        auto val = op_stack[call_stack.back()->bp + instruction.opn_list[0]];
        push_to_stack(val);
        break;
    }

    case Opcode::LOAD_CONST: {
        size_t const_idx = instruction.opn_list[0];
        model::Object* const_val = const_pool[const_idx];
        push_to_stack(const_val);
        break;
    }

    case Opcode::LOAD_BUILTINS: {
        auto obj = builtins[ instruction.opn_list[0] ];
        push_to_stack(obj);
        break;
    }

    case Opcode::LOAD_FREE_VAR: {
        auto func = dynamic_cast<model::Function*>(call_stack.back()->owner);
        assert(func != nullptr);
        push_to_stack(func->free_vars[ instruction.opn_list[0] ]);
        break;
    }

    case Opcode::SET_LOCAL: {
        model::Object* value = get_and_pop_stack_top();

        size_t offset = call_stack.back()->bp + instruction.opn_list[0];
        auto new_val = model::copy_if_mutable(value);
        new_val->make_ref();

        if (op_stack[offset]) {
            op_stack[offset]->del_ref();
        }
        op_stack[offset] = new_val;
        value->del_ref();
        break;
    }


    case Opcode::SET_GLOBAL: {
        auto offset = instruction.opn_list[0];
        auto value = get_and_pop_stack_top();

        auto new_val = model::copy_if_mutable(value);
        new_val->make_ref();
        if (op_stack[offset]) {
            op_stack[offset]->del_ref();
        }

        op_stack[offset] = new_val;
        value->del_ref();
        break;
    }

    case Opcode::SET_NONLOCAL: {
        auto idx_of_upvalue = instruction.opn_list[0];
        auto upvalue = call_stack.back()->code_object->upvalues[ idx_of_upvalue ];
        auto frame = call_stack[ call_stack.size() - upvalue.distance_from_curr - 1]; // 区别于CREATE_CLOSURE指令, 这里在函数中要多减一
        size_t loc_based = frame->bp;

        auto value = get_and_pop_stack_top();

        auto new_val = model::copy_if_mutable(value);
        new_val->make_ref();

        if (op_stack[loc_based + upvalue.idx]) {
            op_stack[loc_based + upvalue.idx]->del_ref();
        }

        op_stack[loc_based + upvalue.idx] = new_val;
        value->del_ref();

        // 更新闭包
        if (auto f = dynamic_cast<model::Function*>(call_stack.back()->owner)) {
            f->free_vars[idx_of_upvalue] = new_val;
        }
        break;
    }


    case Opcode::ENTER_TRY: {
        size_t catch_start = instruction.opn_list[0];
        size_t finally_start = instruction.opn_list[1];
        call_stack.back()->try_blocks.emplace_back(false, catch_start, finally_start);
        break;
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
        break;
    }

    case Opcode::MARK_HANDLE_ERROR: {
        call_stack.back()->try_blocks.back().handle_error = true;
        break;
    }

    case Opcode::THROW: {
        auto top = get_and_pop_stack_top();
        top->del_ref();      // 释放栈引用
        if (call_stack.back()->curr_error) call_stack.back()->curr_error->del_ref();
        call_stack.back()->curr_error = top;
        top->make_ref();     // 使 curr_error 持有引用
        handle_throw();
        break;
    }

    case Opcode::LOAD_ERROR: {
        push_to_stack(call_stack.back()->curr_error);
        break;
    }

    case Opcode::JUMP: {
        size_t target_pc = instruction.opn_list[0];
        call_stack.back()->pc = target_pc;
        break;
    }

    case Opcode::JUMP_IF_FALSE: {
        model::Object* cond = get_and_pop_stack_top();
        if (! is_true(cond)) {
            // 跳转逻辑
            call_stack.back()->pc = instruction.opn_list[0];
        } else {
            call_stack.back()->pc++;
        }
        cond->del_ref();
        break;
    }

    case Opcode::IS_CHILD: {
        auto b = get_and_pop_stack_top();
        auto a = get_and_pop_stack_top();
        push_to_stack(builtin::check_based_object(a, b));
        // 释放使用后的a/b对象，计数对称
        a->del_ref();
        b->del_ref();
        break;
    }

    case Opcode::CREATE_OBJECT: {
        auto obj = new model::Object();
        obj->attrs_insert("__parent__", model::based_obj);
        push_to_stack(obj);
        break;
    }

    case Opcode::IMPORT: {
        std::string module_path = get_attr_name_by_idx(instruction.opn_list[0]);
        handle_import(module_path);
        break;
    }

    case Opcode::CACHE_ITER: {
        auto iter = op_stack.back();
        iter->make_ref();

        call_stack.back()->iters.push_back(iter);
        break;
    }

    case Opcode::GET_ITER: {
        push_to_stack(
            call_stack.back()->iters.back()
        );
        break;
    }

    case Opcode::POP_ITER: {
        auto iter_obj = call_stack.back()->iters.back();
        iter_obj->del_ref();
        call_stack.back()->iters.pop_back();
        break;

    }

    case Opcode::JUMP_IF_FINISH_ITER: {
        auto obj = get_and_pop_stack_top();
        size_t target_pc = instruction.opn_list[0];
        if (obj == model::stop_iter_signal) {
            call_stack.back()->pc = target_pc;
            obj->del_ref(); // 修复：释放使用后的obj
            return;
        }
        call_stack.back()->pc ++;
        obj->del_ref(); // 修复：释放使用后的obj
        break;
    }

    case Opcode::COPY_TOP: {
        auto obj = get_and_pop_stack_top();
        push_to_stack(obj);
        push_to_stack(obj);
        obj->del_ref();
        break;
    }

    case Opcode::STOP: {
        running = false;
        break;
    }

    default: throw NativeFuncError("FutureError", "execute_instruction meet unknown opcode");
    }

}
}
