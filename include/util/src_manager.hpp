/**
 * @file src_manager.hpp
 * @brief 源文件管理器（Source File Manager）核心定义
 * 管理Kiz代码源文件
 * 并给予错误报告器源文件错误代码的切片
 * @author azhz1107cat
 */

#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>

#include "hashmap.hpp"

namespace fs = std::filesystem;

namespace err {

class SrcManager {
public:
    // key: 文件路径, value: 文件内容
    static dep::HashMap<std::string> opened_files;

    // 获取切片
    static std::string get_slice(const std::string& src_path, const int& src_line_start, const int& src_line_end);

    // 获取opened_files中的文件(线程安全)
    static std::string get_file_by_path(std::string path);

    // 打开kiz文件并将其添加到opened_files, 返回文件内容
    static std::string open_new_file(const std::string& path);
};

} // namespace err