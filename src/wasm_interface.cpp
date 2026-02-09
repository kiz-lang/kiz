/**
 * @file wasm_interface.cpp
 * @brief WASM接口文件 - 简化版本
 *
 * 提供JavaScript可以调用的C接口
 */

#include "ir_gen/ir_gen.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "util/src_manager.hpp"
#include "vm/vm.hpp"

#include <string>
#include <cstring>

// 简单的全局状态
static std::string current_path = "<playground#>";
static kiz::Vm global_vm{current_path};

// 在C++中处理，避免复杂的模板问题
extern "C" {

    // 运行代码并返回结果
    const char* run_code(const char* code, int length) {
        static std::string result;

        try {

            std::string content(code, length);
            kiz::Lexer lexer(current_path);
            kiz::Parser parser(current_path);
            kiz::IRGenerator ir_gen(current_path);

            lexer.prepare(content);
            const auto tokens = lexer.tokenize();
            auto ast = parser.parse(tokens);
            const auto ir = ir_gen.gen(std::move(ast));
            const auto module = kiz::IRGenerator::gen_mod(current_path, ir);

            // 这里需要根据实际的VM实现调整
            kiz::Vm::set_main_module(module);
            kiz::Vm::exec_curr_code();

            result = "Success";
        } catch (const std::exception& e) {
            result = std::string("Error: ") + e.what();
        } catch (...) {
            result = "Unknown error";
        }

        return result.c_str();
    }

    // 获取最后一条错误信息
    const char* get_last_error() {
        static std::string error;
        // 这里需要从你的VM中获取错误信息
        return error.c_str();
    }

    // 分配内存（给JavaScript使用）
    char* allocate_memory(int size) {
        return new char[size];
    }

    // 释放内存（给JavaScript使用）
    void free_memory(char* ptr) {
        delete[] ptr;
    }
}