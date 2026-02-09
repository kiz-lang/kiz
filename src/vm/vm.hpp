/**
 * @file vm.hpp
 * @brief 虚拟机(VM)核心定义
 * 执行IR
 * @author azhz1107cat
 * @date 2025-10-25
 */
#pragma once

#include <cassert>

#include "../../deps/hashmap.hpp"

#include <stack>
#include <tuple>
#include <utility>

#include "../kiz.hpp"
#include "../error/error_reporter.hpp"
namespace model {
class Module;
class CodeObject;
class Object;
class List;
class Int;
class Error;
}

namespace kiz {

enum class Opcode : uint8_t;

struct Instruction {
    Opcode opc;
    std::vector<size_t> opn_list;
    err::PositionInfo pos{};
    Instruction(Opcode o, std::vector<size_t> ol, err::PositionInfo& p) : opc(o), opn_list(std::move(ol)), pos(p) {}
};

struct TryFrame {
    bool handle_error = false;
    size_t catch_start = 0;
    size_t finally_start = 0;
};

struct CallFrame {
    std::string name;

    model::Object* owner;
    dep::HashMap<model::Object*> locals;

    size_t pc = 0;
    size_t return_to_pc;
    model::CodeObject* code_object;
    
    std::vector<TryFrame> try_blocks;

    std::vector<model::Object*> iters;

    ~CallFrame();
};

class Vm {
public:
    static dep::HashMap<model::Module*> loaded_modules;
    static model::Module* main_module;

    static std::stack<model::Object*> op_stack;
    static std::vector<std::shared_ptr<CallFrame>> call_stack;
    static model::Int* small_int_pool[201];
    static std::vector<model::Object*> const_pool;

    static dep::HashMap<model::Object*> builtins;

    static bool running;
    static std::string file_path;

    static model::Object* curr_error;

    static dep::HashMap<model::Object*> std_modules;

    explicit Vm(const std::string& file_path_);
    static void reset();

    static void entry_builtins();
    static void entry_std_modules();
    static void handle_import(const std::string& module_path);

    static void set_main_module(model::Module* src_module);
    static void exec_curr_code();
    static void set_and_exec_curr_code(const model::CodeObject* code_object);

    static void execute_instruction(const Instruction& instruction);
    static std::string obj_to_str(model::Object* for_cast_obj);
    static std::string obj_to_debug_str(model::Object* for_cast_obj);
    static void assert_argc(size_t argc, const model::List* args);

    static CallFrame* fetch_curr_call_frame();
    static model::Object* fetch_one_from_stack_top();
    static void push_to_stack(model::Object* obj);

    static model::Object* get_attr(const model::Object* obj, const std::string& attr);
    static bool is_true(model::Object* obj);

    static void instruction_throw(const std::string& name, const std::string& content);
    static auto gen_pos_info()
        -> std::vector<std::pair<std::string, err::PositionInfo>>;
    static void handle_throw();

    /// 如果新增了调用栈，执行循环仅处理新增的模块栈帧（call_stack.size() > old_stack_size），不影响原有调用栈
    static void call_function(model::Object* func_obj, model::Object* args_obj, model::Object* self);
    /// 运算符与普通方法分规则查找
    static void call_method(model::Object* obj, const std::string& attr_name, model::List* args);
    static void execute_unit(const Instruction& instruction);

private:
    /// 如果用户函数则创建调用栈，如果内置函数则执行并压上返回值
    static void handle_call(model::Object* func_obj, model::Object* args_obj, model::Object* self);

    static void exec_ADD(const Instruction& instruction);
    static void exec_SUB(const Instruction& instruction);
    static void exec_MUL(const Instruction& instruction);
    static void exec_DIV(const Instruction& instruction);
    static void exec_MOD(const Instruction& instruction);
    static void exec_POW(const Instruction& instruction);
    static void exec_NEG(const Instruction& instruction);
    static void exec_EQ(const Instruction& instruction);
    static void exec_GT(const Instruction& instruction);
    static void exec_LT(const Instruction& instruction);
    static void exec_GE(const Instruction& instruction);
    static void exec_LE(const Instruction& instruction);
    static void exec_NE(const Instruction& instruction);
    static void exec_NOT(const Instruction& instruction);
    static void exec_IS(const Instruction& instruction);
    static void exec_IN(const Instruction& instruction);

    static void exec_MAKE_LIST(const Instruction& instruction);
    static void exec_MAKE_DICT(const Instruction& instruction);
    static void exec_CALL(const Instruction& instruction);
    static void exec_RET(const Instruction& instruction);
    static void exec_GET_ATTR(const Instruction& instruction);
    static void exec_SET_ATTR(const Instruction& instruction);
    static void exec_GET_ITEM(const Instruction& instruction);
    static void exec_SET_ITEM(const Instruction& instruction);
    static void exec_CALL_METHOD(const Instruction& instruction);
    static void exec_LOAD_VAR(const Instruction& instruction);
    static void exec_LOAD_CONST(const Instruction& instruction);
    static void exec_SET_GLOBAL(const Instruction& instruction);
    static void exec_SET_LOCAL(const Instruction& instruction);
    static void exec_SET_NONLOCAL(const Instruction& instruction);

    static void exec_ENTER_TRY(const Instruction& instruction);
    static void exec_LOAD_ERROR(const Instruction& instruction);
    static void exec_JUMP_IF_FINISH_HANDLE_ERROR(const Instruction& instruction);
    static void exec_MARK_HANDLE_ERROR(const Instruction& instruction);
    static void exec_THROW(const Instruction& instruction);
    static void exec_IMPORT(const Instruction& instruction);

    static void exec_CACHE_ITER(const Instruction& instruction);
    static void exec_GET_ITER(const Instruction& instruction);
    static void exec_POP_ITER(const Instruction& instruction);
    static void exec_JUMP_IF_FINISH_ITER(const Instruction& instruction);

    static void exec_JUMP(const Instruction& instruction);
    static void exec_JUMP_IF_FALSE(const Instruction& instruction);
    static void exec_IS_CHILD(const Instruction& instruction);
    static void exec_CREATE_OBJECT(const Instruction& instruction);
    static void exec_STOP(const Instruction& instruction);
    static void exec_COPY_TOP(const Instruction& instruction);
};

} // namespace kiz