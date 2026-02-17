/**
 * @file repl.cpp
 * @brief 交互式终端（Read Evaluate Print Loop，简称 REPL）核心实现
 *
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "repl.hpp"

#include "../lexer/lexer.hpp"
#include "../vm/vm.hpp"
#include "../ir_gen/ir_gen.hpp"
#include "../parser/parser.hpp"
#include "../kiz.hpp"
#include "color.hpp"
#include "../util/src_manager.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief 控制台编码自适应转换为UTF-8字符串
 * @param src 从控制台getline读取的原始字符串（GBK/UTF-8）
 * @return 转换后的UTF-8字符串，非GBK控制台直接返回原字符串
 */
std::string to_utf8str(std::string src) {
    // 非Windows平台（Linux/macOS）默认控制台是UTF-8，直接返回原字符串
#ifndef _WIN32
    return src;
#else
    // Windows平台：获取控制台输入编码（GetConsoleCP：Console Code Page）
    UINT console_cp = GetConsoleCP();
    // 控制台编码不是GBK（GBK对应Windows代码页936），直接返回原字符串
    if (console_cp != 936) {
        return src;
    }

    // ------------ 核心逻辑：GBK（多字节）→ UTF-16（宽字符）→ UTF-8（多字节） ------------
    // Windows系统编码转换需通过「多字节↔宽字符」中转，使用系统原生API保证兼容性
    int wchar_len = MultiByteToWideChar(
        console_cp,        // 源编码：GBK（936）
        0,                 // 转换标志：0（默认，不做特殊处理）
        src.c_str(),       // 源字符串首地址
        -1,                // 源字符串长度：-1（自动计算，包含末尾的'\0'）
        nullptr,           // 目标宽字符缓冲区：先传nullptr获取所需缓冲区大小
        0                  // 目标缓冲区大小：0（配合nullptr使用）
    );
    // 第一步转换失败：获取宽字符缓冲区大小失败
    if (wchar_len == 0) {
        DWORD err_code = GetLastError();
        throw KizStopRunningSignal("GBK to UTF-16 convert failed, error code: " + std::to_string(err_code));
    }

    // 分配UTF-16宽字符缓冲区（使用vector自动管理内存，避免手动new/delete泄漏）
    std::vector<wchar_t> wchar_buf(wchar_len);
    // 执行GBK→UTF-16实际转换
    int convert_res = MultiByteToWideChar(
        console_cp,
        0,
        src.c_str(),
        -1,
        wchar_buf.data(),  // 目标宽字符缓冲区首地址
        wchar_len          // 目标缓冲区大小（已提前计算）
    );
    // 第二步转换失败：实际GBK→UTF-16转换失败
    if (convert_res == 0) {
        DWORD err_code = GetLastError();
        throw KizStopRunningSignal("GBK to UTF-16 convert failed, error code: " + std::to_string(err_code));
    }

    // UTF-16→UTF-8转换：获取UTF-8缓冲区所需大小
    int utf8_len = WideCharToMultiByte(
        CP_UTF8,           // 目标编码：UTF-8（系统预定义常量CP_UTF8）
        0,                 // 转换标志：0（默认）
        wchar_buf.data(),  // 源宽字符字符串首地址
        -1,                // 源字符串长度：-1（自动计算）
        nullptr,           // 目标UTF-8缓冲区：先传nullptr获取大小
        0,                 // 目标缓冲区大小：0
        nullptr,           // 无效字符替换：nullptr（使用系统默认替换符）
        nullptr            // 替换标志：nullptr
    );
    // 第三步转换失败：获取UTF-8缓冲区大小失败
    if (utf8_len == 0) {
        DWORD err_code = GetLastError();
        throw KizStopRunningSignal("UTF-16 to UTF-8 convert failed, error code: " + std::to_string(err_code));
    }

    // 分配UTF-8缓冲区
    std::vector<char> utf8_buf(utf8_len);
    // 执行UTF-16→UTF-8实际转换
    convert_res = WideCharToMultiByte(
        CP_UTF8,
        0,
        wchar_buf.data(),
        -1,
        utf8_buf.data(),   // 目标UTF-8缓冲区首地址
        utf8_len,
        nullptr,
        nullptr
    );
    // 第四步转换失败：实际UTF-16→UTF-8转换失败
    if (convert_res == 0) {
        DWORD err_code = GetLastError();
        throw KizStopRunningSignal("UTF-16 to UTF-8 convert failed, error code: " + std::to_string(err_code));
    }

    // 将vector缓冲区转换为std::string（自动剔除末尾的'\0'）
    return std::string(utf8_buf.data());
#endif
}

namespace ui {

const std::string Repl::file_path = "<shell#>";

Repl::Repl(): is_running_(true), multiline_start_(1), vm_(file_path) {
    std::cout << "This is the kiz REPL " << "v" << KIZ_VERSION << "\n" << std::endl;
}

std::string Repl::read(const std::string& prompt) {
    std::cout << Color::BRIGHT_MAGENTA << prompt << Color::RESET;
    std::cout.flush();
    std::string result;
    #ifdef _WIN32
        result = to_utf8str(get_whole_input(&std::cin, &std::cout));
    #else
        result = get_whole_input(&std::cin, &std::cout);
    #endif
    return result;
}

void Repl::loop() {
    DEBUG_OUTPUT("start repl loop");
    while (is_running_) {
        try {
            auto code = read(">>> "); // code 可能为多行
            DEBUG_OUTPUT("loop got input: " << code);
            auto old_code_iterator = err::SrcManager::opened_files.find(file_path);
            if (old_code_iterator != err::SrcManager::opened_files.end()) {
                err::SrcManager::opened_files[file_path] = old_code_iterator->second + "\n" + code;
            } else {
                err::SrcManager::opened_files[file_path] = code;
            }
            handle_user_input(code);
        } catch (...) {}
    }
}

void Repl::handle_user_input(const std::string& cmd) {
    // add to history
    DEBUG_OUTPUT("Adding multiline_start_, now it is: " << multiline_start_ << ", cmd_history_.size(): " << cmd_history_.size());

    // 这里要计算实际的行数
    // 在末尾添加换行符以使得slice_file_content可以识别最后一行
    auto actual_additional_line_cnt = (err::slice_file_content(cmd + "\n")).size();
    DEBUG_OUTPUT("actual_additional_line_cnt: " << actual_additional_line_cnt);

    for (const auto &string_this : cmd_history_) {
        DEBUG_OUTPUT("curr: " << string_this);
    } // 似乎原来的cmd_history_永远视为一次一行，所以每一次行数加1

    for (const auto &per_cmd : cmd)
        cmd_history_.emplace_back(&per_cmd);

    // eval and print
    multiline_start_ += actual_additional_line_cnt;
    DEBUG_OUTPUT("After adding, multiline_start_: " << multiline_start_);
    eval_and_print(cmd, multiline_start_ - actual_additional_line_cnt);
    //因为抛出错误会导致后面代码不执行，所以应当先设定multiline_start_再减掉传给它
}

void Repl::eval_and_print(const std::string& cmd, const size_t startline) {
    DEBUG_OUTPUT("repl eval_and_print...");
    bool should_print = false;

    // init
    kiz::Parser parser(file_path);
    kiz::IRGenerator ir_gen(file_path);

    kiz::Lexer lexer(file_path);
    lexer.prepare(cmd, startline);
    const auto tokens = lexer.tokenize();

    auto ast = parser.parse(tokens);
    if (!ast->statements.empty() and
        dynamic_cast<kiz::ExprStmt*>(ast->statements.back().get())
    )   should_print = true;

    const auto ir = ir_gen.gen(std::move(ast), last_global_var_names_);
    last_global_var_names_ = ir_gen.get_global_var_names();

    if (kiz::Vm::call_stack.empty()) {
        const auto module = kiz::IRGenerator::gen_mod(file_path, ir);
        kiz::Vm::set_main_module(module);
    } else {
        if (!ir) throw KizStopRunningSignal("No ir for run" );
        kiz::Vm::reset_global_code(ir);
    }

    DEBUG_OUTPUT("repl print");
    auto stack_top = kiz::Vm::op_stack.back();
    if (stack_top != nullptr) {
        if (not dynamic_cast<model::Nil*>(stack_top) and should_print) {
            std::cout << kiz::Vm::obj_to_debug_str(stack_top) << std::endl;
        }
    }
}

} // namespace repl