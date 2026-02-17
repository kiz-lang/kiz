#include <ranges>

#include "vm.hpp"
#include "builtins/include/builtin_functions.hpp"
#include "opcode/opcode.hpp"

namespace kiz {

// -------------------------- 异常处理 --------------------------

void Vm::forward_to_handle_throw(const std::string& name, const std::string& content) {
    const auto err_name = new model::String(name);
    const auto err_msg = new model::String(content);
    const auto err_obj = new model::Error(make_pos_info());

    err_obj->attrs_insert("__name__", err_name);
    err_obj->attrs_insert("__msg__", err_msg);

    // 替换全局curr_error前，释放旧错误对象
    if (call_stack.back()->curr_error) {
        call_stack.back()->curr_error->del_ref();
    }
    err_obj->make_ref();
    call_stack.back()->curr_error = err_obj;
    handle_throw();
}

void Vm::handle_throw() {
    assert(call_stack.back()->curr_error);

    // 提取错误对象的 __name__ 和 __msg__
    auto err = call_stack.back()->curr_error;
    err->make_ref();
    auto err_name_it = err->attrs.find("__name__");
    auto err_msg_it = err->attrs.find("__msg__");

    if (!err_name_it or !err_msg_it) {
        throw KizStopRunningSignal(
            "Undefined attribute '__name__' '__msg__' of " + obj_to_debug_str(err) + " (when try to throw it)");
    }
    auto error_name = obj_to_str(err_name_it->value);
    auto error_msg = obj_to_str(err_msg_it->value);

    size_t frames_to_pop = 0; // 需要从栈顶弹出的帧数

    // 执行ensure确保资源被释放
    handle_ensure();

    // 逆序遍历调用栈
    for (auto frame : std::ranges::reverse_view(call_stack)) {
        for (const auto& table : frame->code_object->exception_tables) {
            // 检查当前 pc 是否在此 try 块的范围内
            size_t current_pc = frame->pc;
            if (table.try_part_start_pc <= current_pc and current_pc < table.try_part_end_pc) {
                // 寻找匹配的 catch 块
                if (auto catch_start_pc_it = table.handle_pc.find(error_name)) {
                    frame->pc = catch_start_pc_it->value;
                } else {
                    frame->pc = table.mismatch_pc;
                }

                // 弹出多余的栈帧
                for (size_t i = 0; i < frames_to_pop; ++i) {
                    call_stack.pop_back();
                }
                err->make_ref();
                frame->curr_error = err;
                return;
            }
        }
        ++frames_to_pop;
    }

    // 没有找到任何能处理该异常的 try 块：打印错误信息并终止执行
    if (const auto err_obj = dynamic_cast<model::Error*>(err)) {
        std::cout << Color::BRIGHT_RED << "\nTrace Back: " << Color::RESET << std::endl;
        for (auto& [_path, _pos] : err_obj->positions) {
            err::context_printer(_path, _pos);
        }
    }

    std::cout << Color::BOLD <<
        Color::BRIGHT_RED << error_name << Color::RESET
        << Color::WHITE << " : " << error_msg << Color::RESET << "\n";

    std::cout << std::endl;

    err->del_ref();
    call_stack.back()->curr_error = nullptr;
    throw KizStopRunningSignal();
}

void Vm::handle_ensure() {
    auto frame = call_stack.back();
    if (frame->exec_ensure_stmt) return;

    auto ensures = frame->code_object->ensure_stmts;
    if (ensures.empty()) {
        return;
    }

    auto old_code = frame->code_object->code;
    size_t old_pc = call_stack.back()->pc;
    frame->code_object->code = ensures;
    frame->pc = 0;

    while (!call_stack.empty() && running) {
        auto curr_frame = call_stack.back();
        // 检查当前帧是否执行完毕
        if (curr_frame->pc >= curr_frame->code_object->code.size()) {
            break;
        }

        // 执行当前指令
        const Instruction& curr_inst = curr_frame->code_object->code[curr_frame->pc];
        try {
            execute_unit(curr_inst);
        } catch (NativeFuncError& e) {
            forward_to_handle_throw(e.name, e.msg);
            continue;
        }

        ADVANCE_PC
    }
    frame->code_object->code = old_code;
    frame->pc = old_pc;
    frame->exec_ensure_stmt = true;
}

}
