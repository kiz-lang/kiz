/**
 * @file error_reporter.hpp
 * @brief 错误报告器（Error Reporter）核心定义
 * kiz错误报告器
 * @author azhz1107cat
 */

#pragma once
#include <string>
#include <vector>

#include "hashmap.hpp"

namespace err {

struct PositionInfo {
    // std::string file_path;
    size_t lno_start;
    size_t lno_end;
    size_t col_start;
    size_t col_end;
};

std::string generate_separator(int col_start, int col_end, int line_end);

void error_reporter(
    const std::string& src_path,
    const PositionInfo& pos,
    const std::string& error_name,
    const std::string& error_content
);


void context_printer(
    const std::string& src_path,
    const PositionInfo& pos
);

}// namespace err