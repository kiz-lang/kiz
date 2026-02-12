#include "include/io_lib.hpp"
#include <fstream>

#include "builtins/include/builtin_functions.hpp"

namespace io_lib {

std::streamsize util_write(
    const std::string& path,
    const std::string& text,
    std::streamoff n = 0
) {
    // 空文本直接返回，不执行任何写入操作
    if (text.empty()) {
        return 0;
    }

    // 关键打开模式：in+out+binary → 可读可写+二进制模式
    // in+out：避免默认截断文件（out单独使用会清空文件），同时支持文件不存在时的后续创建
    // binary：禁止换行符自动转换，保证偏移量和写入内容的字节级精准，跨平台一致
    std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);

    // 若文件不存在，重新以"创建+可写+二进制"模式打开（覆盖上述失败的打开）
    if (!file.is_open()) {
        file.open(path, std::ios::trunc | std::ios::out | std::ios::binary);
        // 二次打开失败，抛出带路径的异常（权限不足/路径非法等）
        if (!file.is_open()) {
            throw NativeFuncError("IOError","Failed to open/create file: " + path);
        }
    }

    try {
        // 将文件写入指针移动到偏移量n的位置（从文件开头开始计算）
        // seekp：输出流的位置指针操作（put pointer），适配写入操作
        file.seekp(n, std::ios::beg);
        // 校验偏移是否成功（如n为负数、超出系统最大文件偏移量等情况）
        if (file.tellp() != n) {
            throw NativeFuncError("IOError","Failed to seek to offset: " + std::to_string(n));
        }

        // 处理偏移量超出文件大小的情况：自动填充0补全空隙，避免文件空洞
        std::streamoff file_size = 0;
        file.seekg(0, std::ios::end); // 移动读取指针到文件末尾，获取文件总大小
        file_size = file.tellg();    // tellg()返回文件末尾的偏移量，即文件大小
        if (n > file_size) {
            std::streamoff gap = n - file_size; // 计算需要填充的0的字节数
            file.seekp(file_size, std::ios::beg); // 移动到原文件末尾
            // 循环填充0，避免单次填充超大空隙导致的性能问题
            const std::streamsize fill_block = 4096;
            char zero = 0;
            for (std::streamoff i = 0; i < gap; ++i) {
                file.write(&zero, 1);
            }
            // 填充后重新移动到偏移量n的位置
            file.seekp(n, std::ios::beg);
        }

        // 执行实际写入：将text的原始字节写入文件
        file.write(text.data(), text.size());
        // 强制刷新缓冲区：确保数据立即写入磁盘，避免缓冲区未刷新导致的写入丢失
        file.flush();

        // 校验写入是否成功：检测流的错误状态
        if (file.fail()) {
            throw NativeFuncError("IOError", "File write failed at offset: " + std::to_string(n));
        }

        // 返回实际写入的字节数
        return text.size();
    } catch (const std::exception& e) {
        throw NativeFuncError("IOError","File: " + path + ", " + e.what());
    }
}

model::Object* init_module(model::Object* self, const model::List* args) {
    auto mod = new model::Module("io_lib");

    auto fast_read_fun = model::create_nfunc(fast_read);
    auto fast_write_fun = model::create_nfunc(fast_write);

    mod->attrs_insert("fast_read", fast_read_fun);
    mod->attrs_insert("fast_write", fast_write_fun);

    return mod;
}

model::Object* fast_read(model::Object* self, const model::List* args) {
    auto path = builtin::get_one_arg(args);
    auto path_str = dynamic_cast<model::String*>(path);
    if (! path_str)
        throw NativeFuncError("TypeError", "io.fast_read only supports one String type argument");

    std::ifstream file(path_str->val, std::ios::binary | std::ios::in);

    if (!file.is_open()) {
        throw NativeFuncError("PathError", "Failed to open file: " + path_str->val);
    }

    auto content = std::string(std::istreambuf_iterator(file),
                       std::istreambuf_iterator<char>()
    );
    auto str_obj = new model::String(content);
    return str_obj;
}

model::Object* fast_write(model::Object* self, const model::List* args) {
    auto args_vec = args->val;
    kiz::Vm::assert_argc(3, args);

    auto path_str = dynamic_cast<model::String*>(args_vec[0]);
    if (! path_str)
        throw NativeFuncError("TypeError", "io.fast_write[0] supports String type argument");

    auto text_str = dynamic_cast<model::String*>(args_vec[1]);
    if (! text_str)
        throw NativeFuncError("TypeError", "io.fast_write[1] supports String type argument");

    auto start_idx = dynamic_cast<model::Int*>(args_vec[2]);
    if (! start_idx)
        throw NativeFuncError("TypeError", "io.fast_write[2] supports Int type argument");

    util_write(path_str->val, text_str->val, start_idx->val.to_unsigned_long_long());

    return model::load_nil();
}

}
