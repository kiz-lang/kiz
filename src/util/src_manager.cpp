/**
 * @file src_manager.cpp
 * @brief 源文件管理器（Source File Manager）核心
 * 管理Kiz代码源文件
 * 并给予错误报告器源文件错误代码的切片
 * @author azhz1107cat
 */

#include "util/src_manager.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "kiz.hpp"

namespace err {

dep::HashMap<std::string> SrcManager::opened_files;
inline std::mutex opened_files_mutex; // 全局锁

/**
 * @brief 从指定文件中提取指定行范围的内容（行号从1开始）
 * @param src_path 文件路径
 * @param src_line_start 起始行号（闭区间）
 * @param src_line_end 结束行号（闭区间）
 * @return 提取的行内容（每行保留原始换行符，无效范围返回空字符串）
 */
std::string SrcManager::get_slice(const std::string& src_path, const int& src_line_start, const int& src_line_end) {
    DEBUG_OUTPUT("get slice");
    // 先获取完整文件内容（依赖缓存机制）
    std::string file_content = get_file_by_path(src_path);
    if (file_content.empty()) {
        return "";
    }

    DEBUG_OUTPUT("try to slice");
    // 按行分割文件内容（兼容 Windows \r\n 和 Linux \n 换行符）
    std::vector<std::string> lines;
    std::stringstream content_stream(file_content);
    std::string line;
    while (std::getline(content_stream, line)) {
        // 移除 Windows 换行符残留的 \r（std::getline 会自动去掉 \n，但不会处理 \r）
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.emplace_back(std::move(line));
    }

    // 校验行号范围有效性（行号从1开始，且起始行 ≤ 结束行）
    const size_t total_lines = lines.size();
    if (src_line_start < 1 || src_line_end < 1 || 
        src_line_start > src_line_end || 
        static_cast<size_t>(src_line_end) > total_lines
    ) {
        // std::cerr << "[Warning] Invalid line range: start=" << src_line_start
        //          << ", end=" << src_line_end << " (total lines: " << total_lines << ")\n";
        return "";
    }

    // 提取目标行范围（转换为0-based索引）
    std::stringstream slice_stream;
    const size_t start_idx = static_cast<size_t>(src_line_start) - 1;
    const size_t end_idx = static_cast<size_t>(src_line_end) - 1;
    for (size_t i = start_idx; i <= end_idx; ++i) {
        slice_stream << lines[i];
        // 除最后一行外，保留原始换行符（还原文件格式）
        if (i != end_idx) {
            slice_stream << '\n';
        }
    }

    return slice_stream.str();
}

/**
 * @brief 线程安全获取文件内容（优先从缓存读取，未命中则新打开）
 * @param path 文件路径
 * @return 文件完整内容（打开失败会抛出异常）
 */
std::string SrcManager::get_file_by_path(std::string path) {
    DEBUG_OUTPUT("get_file_by_path");
    std::lock_guard lock(opened_files_mutex); // 加锁
    // 检查缓存是否已存在该文件
    auto iter = opened_files.find(path);
    if (iter != nullptr) {
        return iter->value;
    }

    // 缓存未命中，新打开文件并加入缓存
    std::string file_content = open_new_file(path);
    DEBUG_OUTPUT(file_content);
    DEBUG_OUTPUT(path+" "+file_content);
    opened_files.insert(path, file_content);
    DEBUG_OUTPUT("finish get_file_by_path");
    return file_content;
}

/**
 * @brief 打开Kiz文件并读取内容（不直接对外暴露，由get_file_by_path调用）
 * @param path 文件路径
 * @return 文件完整内容（打开失败会抛出std::runtime_error）
 */
std::string SrcManager::open_new_file(const std::string& path) {
    DEBUG_OUTPUT("open_new_file: " + path);
    std::ifstream kiz_file(path, std::ios::binary);
    if (!kiz_file.is_open()) {
        throw KizStopRunningSignal("Failed to open file: " + path);
    }

    kiz_file.seekg(0, std::ios::end);
    const std::streampos file_size = kiz_file.tellg();
    if (file_size < 0) {
        throw KizStopRunningSignal("Failed to get file size: " + path);
    }

    std::string file_content;
    file_content.resize(file_size);

    kiz_file.seekg(0, std::ios::beg);
    kiz_file.read(&file_content[0], file_size);

    if (!kiz_file) {
        throw KizStopRunningSignal("Failed to read file: " + path);
    }

    DEBUG_OUTPUT("File read successfully, size: " + std::to_string(file_size));
    DEBUG_OUTPUT(file_content);
    return file_content;
}

} // namespace err