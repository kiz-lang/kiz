/**
 * @file error_reporter.cpp
 * @brief 错误报告器（Error Reporter）核心实现
 * kiz错误报告器
 * @author azhz1107cat
 */

#include "error_reporter.hpp"

#include "../util/src_manager.hpp"

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

    // 获取错误行代码
    DEBUG_OUTPUT("getting line. Additional, in context_printer, reporting an error. Pos: line " << pos.lno_start
        << "~" << pos.lno_end << ", col "
        << pos.col_start << "~" << pos.col_end << ". src_path: " << src_path);

    std::string error_line = SrcManager::get_slice(src_path, src_line_start, src_line_end);
    DEBUG_OUTPUT(error_line);
    // 只有当切片范围无效时才显示错误信息，空行是有效的
    bool is_valid_range = src_line_start >= 1 && src_line_end >= 1 && src_line_start <= src_line_end;
    if (error_line.empty() && !is_valid_range) {
        error_line = "[Can't slice the source file with "
        + std::to_string(src_line_start) + "," + std::to_string(src_line_start)
        + "," + std::to_string(src_col_start) + "," + std::to_string(src_col_end) + "]";
    }

    DEBUG_OUTPUT("making error pointer ( ^ )");
    // 计算箭头位置：行号前缀长度 + 列偏移（列从1开始）
    const std::string line_prefix = std::to_string(src_line_start) + " | ";
    const size_t caret_offset = line_prefix.size() + (src_col_start - 1);
    // 生成箭头（单个^或连续^，匹配错误列范围）
    const std::string caret = std::string(src_col_end - src_col_start + 1, '^');

    // 格式化输出（颜色高亮+固定格式）
    std::cout << std::endl;
    // 文件路径
    std::cout << Color::BRIGHT_BLUE << "File \"" << src_path << "\"" << Color::RESET << std::endl;
    // 行号 + 错误代码行
    std::cout << Color::WHITE << line_prefix << error_line << Color::RESET << std::endl;
    // 箭头（对准错误列）
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