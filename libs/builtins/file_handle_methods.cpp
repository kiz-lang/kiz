#include "../../src/models/models.hpp"

namespace model {

Object* file_handle_read(Object* self, const List* args) {
    kiz::Vm::assert_argc(0, args);
    auto f_obj = dynamic_cast<FileHandle*>(self);
    assert(f_obj);

    if (f_obj->is_closed) {
        throw NativeFuncError("FileError", "Cannot read from closed file handle");
    }
    if (!f_obj->file_handle || !f_obj->file_handle->good()) {
        throw NativeFuncError("FileError", "Invalid or corrupted file handle");
    }

    // 关键修复：先清除流的错误状态（比如 EOF），再移动读指针
    f_obj->file_handle->clear();
    // 移动读指针到文件开头（追加模式写入后指针在末尾，需重置）
    f_obj->file_handle->seekg(0, std::ios::beg);
    // 同步写指针（避免读写指针不一致）
    f_obj->file_handle->seekp(0, std::ios::end);

    std::ostringstream oss;
    oss << f_obj->file_handle->rdbuf();

    return new String(oss.str());
}

Object* file_handle_write(Object* self, const List* args) {
    kiz::Vm::assert_argc(1, args);

    // 类型转换并校验
    auto f_obj = dynamic_cast<FileHandle*>(self);
    assert(f_obj);

    // 校验文件句柄状态
    if (f_obj->is_closed) {
        throw NativeFuncError("FileError", "Cannot write to closed file handle");
    }
    if (!f_obj->file_handle || !f_obj->file_handle->good()) {
        throw NativeFuncError("FileError", "Invalid or corrupted file handle");
    }

    // 提取要写入的字符串内容
    std::string content = kiz::Vm::obj_to_str(args->val[0]);

    // 写入内容并刷新缓冲区
    *f_obj->file_handle << content;
    f_obj->file_handle->flush();

    // 返回空对象（无返回值）
    return load_nil();
}

Object* file_handle_readline(Object* self, const List* args) {
    kiz::Vm::assert_argc(1, args);

    // 类型转换并校验 FileHandle 实例
    auto f_obj = dynamic_cast<FileHandle*>(self);
    assert(f_obj);

    // 校验文件句柄状态
    if (f_obj->is_closed) {
        throw NativeFuncError("FileError", "Cannot read from closed file handle");
    }
    if (!f_obj->file_handle || !f_obj->file_handle->good()) {
        throw NativeFuncError("FileError", "Invalid or corrupted file handle");
    }

    // 提取并校验行号参数
    auto lineno_obj = cast_to_int(args->val[0]);
    int64_t lineno = lineno_obj->val.to_unsigned_long_long();

    // 行号必须 >=1，否则返回空字符串
    if (lineno < 1) {
        throw NativeFuncError("ValueError", "lineno must be >= 1 (got " + std::to_string(lineno) + ")");
    }

    // 读取指定行的核心逻辑
    std::string target_line; // 存储目标行内容
    std::string current_line; // 临时存储当前行
    int64_t current_linno = 0; // 当前行号（从0开始计数）

    // 重置文件读取指针到开头，确保每次读取都是从文件起始位置开始
    f_obj->file_handle->seekg(0, std::ios::beg);
    f_obj->file_handle->clear(); // 清除之前的 EOF 等状态

    // 逐行读取，直到找到目标行或文件结束
    while (std::getline(*f_obj->file_handle, current_line)) {
        current_linno++;
        if (current_linno == lineno) {
            target_line = current_line;
            // 补回 get_line 吃掉的换行符（除非是文件最后一行）
            if (!f_obj->file_handle->eof()) {
                target_line += "\n";
            }
            break; // 找到目标行，退出循环
        }
    }

    return new String(target_line);
}

Object* file_handle_close(Object* self, const List* args) {
    kiz::Vm::assert_argc(0, args);

    // 类型转换并校验
    auto f_obj = dynamic_cast<FileHandle*>(self);
    assert(f_obj);

    if (f_obj->is_closed) {
        return load_nil();
    }

    // 关闭文件句柄并释放资源
    if (f_obj->file_handle) {
        f_obj->file_handle->close();
        delete f_obj->file_handle;
        f_obj->file_handle = nullptr;
    }

    // 标记为已关闭
    f_obj->is_closed = true;

    return load_nil();
}

}