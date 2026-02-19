/**
 * @file error_reporter.cpp
 * @brief 错误报告器（Error Reporter）核心实现
 * kiz错误报告器
 * @author azhz1107cat
 */

#include "error_reporter.hpp"

#include "src_manager.hpp"

#include <iostream>
#include <string>

#include "../kiz.hpp"
#include "../repl/color.hpp"

namespace err {
void context_printer(
    const std::string& src_path,
    const PositionInfo& pos
) {
#ifdef __EMSCRIPTEN__
#else
    size_t src_line_start = pos.lno_start;
    size_t src_line_end = pos.lno_end;
    size_t src_col_start = pos.col_start;
    size_t src_col_end = pos.col_end;

    // 获取错误代码片段（可能多行）
    std::string error_slice = SrcManager::get_slice(src_path, src_line_start, src_line_end);
    bool is_valid_range = src_line_start >= 1 && src_line_end >= 1 && src_line_start <= src_line_end;
    if (error_slice.empty() && !is_valid_range) {
        error_slice = "[Can't slice the source file with "
        + std::to_string(src_line_start) + "," + std::to_string(src_line_start)
        + "," + std::to_string(src_col_start) + "," + std::to_string(src_col_end) + "]";
    }

    // 按行分割
    std::vector<std::string> lines = SrcManager::splitlines(error_slice);
    size_t line_count = lines.size();

    std::cout << std::endl;
    std::cout << Color::BRIGHT_BLUE << "File \"" << src_path << "\"" << Color::RESET << std::endl;

    // 打印每一行，带行号前缀
    for (size_t i = 0; i < line_count; ++i) {
        size_t current_lineno = src_line_start + i;
        std::string line_prefix = std::to_string(current_lineno) + " | ";
        std::cout << Color::WHITE << line_prefix << lines[i] << Color::RESET << std::endl;
    }

    // 箭头定位在最后一行下方
    // 计算最后一行的行号前缀长度
    std::string last_line_prefix = std::to_string(src_line_end) + " | ";
    size_t caret_offset = last_line_prefix.size() + (src_col_start - 1); // 使用起始列（通常等于结束列，因为是一个点）
    // 生成箭头（长度基于列范围）
    std::string caret = std::string(src_col_end - src_col_start + 1, '^');
    std::cout << std::string(caret_offset, ' ') << Color::BRIGHT_RED << caret << Color::RESET << std::endl;
#endif
}


void error_reporter(
    const std::string& src_path,
    const PositionInfo& pos,
    const std::string& error_name,
    const std::string& error_content
) {
#ifdef __EMSCRIPTEN__
    std::cout << error_name << ":" << error_content << Color::RESET << std::endl;
#else
    context_printer(src_path, pos);
    // 错误信息（类型加粗红 + 内容白）
    std::cout << Color::BOLD << Color::BRIGHT_RED << error_name
              << Color::RESET << Color::WHITE << " : " << error_content
              << Color::RESET << std::endl;
    std::cout << std::endl;

    throw KizStopRunningSignal();
#endif
}

}// namespace err