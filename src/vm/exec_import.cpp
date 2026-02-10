#include <cassert>
#include <fstream>

#include "../kiz.hpp"
#include "../models/models.hpp"
#include "vm.hpp"
#include "builtins/include/builtin_functions.hpp"
#include "ir_gen/ir_gen.hpp"
#include "lexer/lexer.hpp"
#include "opcode/opcode.hpp"
#include "parser/parser.hpp"
#include "util/src_manager.hpp"

#include <filesystem>
#include <string>
#include <stdexcept>
#include <cstddef>
#include <array>
#include <format>

// 跨平台兼容：处理Windows/Linux/macOS的编译差异
#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <cstdint>
#endif

namespace fs = std::filesystem;

/**
 * @brief 跨平台获取EXE可执行文件的**完整绝对路径**（含exe文件名）
 * @return fs::path EXE的绝对路径（如Windows: C:/project/bin/Debug/app.exe，Linux: /home/user/bin/app）
 */
fs::path get_exe_abs_path() {
    fs::path exe_path;
#ifdef _WIN32
    // Windows平台：使用GetModuleFileNameW获取宽字符路径（避免中文路径乱码）
    wchar_t buf[MAX_PATH] = {0};
    DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    // 处理缓冲区不足的情况，动态扩容
    if (len == MAX_PATH) {
        do {
            std::wstring big_buf(len + MAX_PATH, L'\0');
            len = GetModuleFileNameW(nullptr, big_buf.data(), static_cast<DWORD>(big_buf.size()));
            if (len < big_buf.size()) {
                exe_path = big_buf.substr(0, len);
                break;
            }
        } while (true);
    } else if (len > 0) {
        exe_path = std::wstring(buf, len);
    } else {
        throw NativeFuncError("PathError","Windows GetModuleFileNameW failed, error code: " + std::to_string(GetLastError()));
    }
#elif defined(__linux__)
    // Linux平台：读取/proc/self/exe符号链接（指向当前进程的可执行文件）
    char buf[PATH_MAX] = {0};
    ssize_t len = readlink("/proc/self/exe", buf, PATH_MAX - 1);
    if (len == -1) {
        throw NativeFuncError("PathError", "Linux readlink /proc/self/exe failed");
    }
    exe_path = std::string(buf, len);
#elif defined(__APPLE__)
    // macOS平台：使用_NSGetExecutablePath获取可执行文件路径
    char buf[PATH_MAX] = {0};
    uint32_t buf_len = PATH_MAX;
    int ret = _NSGetExecutablePath(buf, &buf_len);
    // 缓冲区不足时扩容
    if (ret == -1) {
        std::string big_buf(buf_len, '\0');
        ret = _NSGetExecutablePath(big_buf.data(), &buf_len);
        if (ret != 0) {
            throw NativeFuncError("PathError", "macOS _NSGetExecutablePath failed");
        }
        exe_path = big_buf;
    } else {
        exe_path = std::string(buf);
    }
    // macOS需将相对路径转换为绝对路径
    exe_path = fs::absolute(exe_path);
#else
    throw KizStopRunningSignal("Unsupported platform");
#endif

    // 确保返回绝对路径（跨平台兜底）
    return fs::absolute(exe_path);
}

/**
 * @brief 跨平台获取EXE可执行文件的绝对路径所在目录
*/
fs::path get_exe_abs_dir() {
    return get_exe_abs_path().parent_path();
}

/**
 * @brief 路径安全拼接+规范化（自动解析.././、处理分隔符、跨平台）
 * @param base_path 基础路径（如EXE目录、任意绝对/相对路径）
 * @param path_fragments 待拼接的路径片段（支持多个，如"../conf"、"app.json"、"./data/log"）
 * @return fs::path 规范化后的绝对/相对路径（解析了.././，无冗余分隔符）
 * @note 若base_path是绝对路径，返回结果也为绝对路径；反之则为相对路径
 */
template <typename... Args>
fs::path path_combine(const fs::path& base_path, Args&&... path_fragments) {
    fs::path result = base_path;
    (result /= ... /= std::forward<Args>(path_fragments));
    return result.lexically_normal();
}

// ========== 新增：核心函数 get_file_name_by_path ==========
/**
 * @brief 从路径字符串中提取去掉「最后一个扩展名」的纯文件名
 * @param file_path 任意路径（绝对/相对，如"k/l/m.log"、"hhh/k.m.log"、"test.txt"）
 * @return std::string 纯文件名（无路径、无最后一个扩展名）
 * @note 边界情况处理：
 *       1. 无扩展名："a/b/c" → "c"；"test" → "test"
 *       2. 隐藏文件：".gitignore" → ".gitignore"（无最后一个扩展名，保留完整）
 *       3. 多分隔符/末尾分隔符："a//b/c.log/" → ""；"/" → ""
 *       4. 纯扩展名："a/b/.log" → ""
 */
std::string get_file_name_by_path(const std::string& file_path) {
    fs::path path_obj(file_path);
    // 先取文件名（带扩展名），再移除最后一个扩展名，最终转字符串
    return path_obj.filename().stem().string();
}

namespace kiz {

void Vm::exec_IMPORT(const Instruction& instruction) {
    size_t path_idx = instruction.opn_list[0];
    std::string module_path = call_stack.back()->code_object->names[path_idx];
    handle_import(module_path);
}

void Vm::handle_import(const std::string& module_path) {
    std::string content;

    // 先向缓存中查找
    if (auto loaded_mod_it = modules_cache.find(module_path)) {
        call_stack.back()->locals.insert(loaded_mod_it->value->path, loaded_mod_it->value);
        return;
    }

    fs::path current_file_path;
    for (const auto& frame: call_stack) {
        if (frame->owner->get_type() == model::Object::ObjectType::Module) {
            const auto m = dynamic_cast<model::Module*>(frame->owner);
            current_file_path = m->path;
        }
    }

    bool file_in_path = false;
    fs::path actually_found_path = "";
    std::vector for_search_paths = {
        get_exe_abs_dir() / current_file_path.parent_path() / fs::path(module_path),
        get_exe_abs_dir() / fs::path(module_path)
    };

#ifdef __EMSCRIPTEN__
#else
    for (const auto& for_search_path : for_search_paths) {
        if (fs::is_regular_file( for_search_path )) {
            file_in_path = true;
            actually_found_path = for_search_path;
            break;
        }
    }
#endif


    if (file_in_path) {
#ifdef __EMSCRIPTEN__
        // 不可能走到这
#else
        content = err::SrcManager::get_file_by_path(actually_found_path.string());
#endif
    } else if (auto std_init_it = std_modules.find(module_path)) {
        auto std_init_func = dynamic_cast<model::NativeFunction*>(std_init_it->value);
        assert(std_init_func != nullptr);

        // 使用create_list创建临时参数，保存指针
        model::List* args_list = model::cast_to_list(model::create_list({}));
        model::Object* return_val = std_init_func->func(std_init_func, args_list);
        // 使用后释放临时参数列表
        args_list->del_ref();

        assert(return_val != nullptr);
        auto module_obj = dynamic_cast<model::Module*>(return_val);
        assert(module_obj != nullptr);

        // 移除重复的make_ref()，仅一次即可
        module_obj->make_ref();
        call_stack.back()->locals.insert(module_path, module_obj);

        module_obj->make_ref();
        modules_cache.insert(module_path, module_obj);
        return;
    } else {
        throw NativeFuncError("PathError", std::format(
            "Failed to find module in path '{}', tried '{}', '{}'", module_path,
            for_search_paths[0].string(), for_search_paths[1].string()));
    }

    Lexer lexer(module_path);
    Parser parser(module_path);
    IRGenerator ir_gen(module_path);

    lexer.prepare(content);
    const auto tokens = lexer.tokenize();
    auto ast = parser.parse(tokens);
    const auto ir = ir_gen.gen(std::move(ast));
    auto module_obj = IRGenerator::gen_mod(module_path, ir);


    auto new_frame = std::make_shared<CallFrame>(CallFrame{
        module_path,

        module_obj,   // owner
        dep::HashMap<model::Object*>(), // 初始空局部变量表

        0,                               // 程序计数器初始化为0（从第一条指令开始执行）
        call_stack.back()->pc + 1,   // 执行完所有指令后返回的位置（指令池末尾）
        module_obj->code,                 // 关联当前模块的CodeObject

        {}
    });

    size_t old_call_stack_size = call_stack.size();

    call_stack.emplace_back(std::move(new_frame));

    while (running and !call_stack.empty()) {
        auto& curr_frame = *call_stack.back();
        auto& frame_code = curr_frame.code_object;

        // 检查是否执行到模块代码末尾：执行完毕则出栈
        if (curr_frame.pc >= frame_code->code.size()) {
            if (old_call_stack_size == call_stack.size() - 1) {
                break;
            }
            call_stack.pop_back();
            continue;
        }

        // 执行当前指令
        const Instruction& curr_inst = frame_code->code[curr_frame.pc];
        try {
            execute_instruction(curr_inst); // 调用VM的指令执行核心方法
        } catch (const NativeFuncError& e) {
            // 原生函数执行错误，抛出异常
            instruction_throw(e.name, e.msg);
            return;
        } catch (const KizStopRunningSignal& e) {
            // 模块执行中触发停止信号，终止执行
            running = false;
            return;
        }

        if (curr_inst.opc != Opcode::JUMP && curr_inst.opc != Opcode::JUMP_IF_FALSE &&
            curr_inst.opc != Opcode::RET && curr_inst.opc != Opcode::JUMP_IF_FINISH_HANDLE_ERROR
            && curr_inst.opc != Opcode::THROW && curr_inst.opc != Opcode::JUMP_IF_FINISH_ITER) {
            curr_frame.pc++;
            }

    }

    std::string module_name = get_file_name_by_path(module_path); // 默认值
    for (const auto& [name, local_object] : call_stack.back()->locals.to_vector()) {
        if (name.starts_with("__private__")) continue;
        if (name == "__name__") {
            auto module_name_str = dynamic_cast<model::String*>(local_object);
            assert(module_name_str != nullptr);
            module_name = module_name_str->val;
        }
        // 插入__owner_module__前，正确管理module_obj引用
        module_obj->make_ref();
        local_object->attrs.insert("__owner_module__", module_obj);
        module_obj->del_ref(); // 移交所有权给local_object->attrs，计数平衡

        // 插入到module_obj->attrs前，正确管理local_object引用
        local_object->make_ref();
        module_obj->attrs.insert(name, local_object);
        local_object->del_ref(); // 移交所有权给module_obj->attrs，计数平衡
    }

    call_stack.pop_back();

    module_obj->path = module_path;
    module_obj->make_ref();
    call_stack.back()->locals.insert(module_name, module_obj);

    module_obj->make_ref();
    modules_cache.insert(module_path, module_obj);
}

}
