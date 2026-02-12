#include <ranges>

#include "vm.hpp"
#include "builtins/include/builtin_functions.hpp"

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
    call_stack.back()->curr_error = err_obj; // 全局持有
    handle_throw();
}

void Vm::handle_throw() {
    assert(call_stack.back()->curr_error != nullptr);

    size_t frames_to_pop = 0;
    CallFrame* target_frame = nullptr;
    size_t target_pc = 0;

    // 逆序遍历调用栈，寻找最近的 try 块
    for (auto frame : std::ranges::reverse_view(call_stack)) {
        if (!frame->try_blocks.empty()) {
            target_frame = frame;
            auto try_frame = frame->try_blocks.back();
            // std::cout << "try_frame.handle_error: " << try_frame.handle_error << std::endl;
            if (try_frame.handle_error) {
                // 该情况下, error从catch中抛出
                // 改为从 finally 开始执行
                target_pc = try_frame.finally_start;
                // std::cout << "find finally pc!" << target_pc << std::endl;
                // 重置错误处理状态, 使其在finally后可以rethrow
                frame->try_blocks.back().handle_error = false;
                // std::cout << "change try_frame.handle_error: " << frame->try_blocks.back().handle_error << std::endl;
                break;
            } else {
                target_pc = try_frame.catch_start;
                assert(target_pc != 0);
                break;
            }
        }
        frames_to_pop++;
    }

    // 如果找到有效的 try 块
    if (target_frame) {
        // 弹出多余的栈帧
        for (size_t i = 0; i < frames_to_pop; ++i) {
            call_stack.pop_back();
        }
        // 设置 pc
        target_frame->pc = target_pc;
        return;
    }

    auto err_name_it = call_stack.back()->curr_error->attrs.find("__name__");
    auto err_msg_it = call_stack.back()->curr_error->attrs.find("__msg__");
    if (!err_name_it or !err_msg_it) {
        throw NativeFuncError("NameError",
        "Undefined attribute '__name__' '__msg__'  of " + obj_to_debug_str(call_stack.back()->curr_error) + " (when try to throw it)"
        );
    }
    auto error_name = obj_to_str(err_name_it->value);
    auto error_msg = obj_to_str(err_msg_it->value);

    // 报错
    if (const auto err_obj = dynamic_cast<model::Error*>(call_stack.back()->curr_error)) {
        std::cout << Color::BRIGHT_RED << "\nTrace Back: " << Color::RESET << std::endl;
        for (auto& [_path, _pos]: err_obj->positions ) {
            err::context_printer(_path, _pos);
        }
    }

    // 错误信息（类型加粗红 + 内容白）
    std::cout << Color::BOLD << Color::BRIGHT_RED << error_name
              << Color::RESET << Color::WHITE << " : " << error_msg
              << Color::RESET << std::endl;
    std::cout << std::endl;

    call_stack.back()->curr_error->del_ref();
    call_stack.back()->curr_error = nullptr;

    throw KizStopRunningSignal();
}

}