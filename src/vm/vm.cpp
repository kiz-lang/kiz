/**
 * @file vm.cpp
 * @brief 虚拟机（VM）核心实现
 * 用于运行IR
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "vm.hpp"

#include "../models/models.hpp"
#include "../op_code/opcode.hpp"

#include <algorithm>
#include <cassert>

#include "../kiz.hpp"

#include "../libs/builtins/include/builtin_methods.hpp"
#include "../libs/builtins/include/builtin_functions.hpp"

namespace kiz {

dep::HashMap<model::Object*> Vm::builtins{};
dep::HashMap<model::Module*> Vm::loaded_modules{};
model::Module* Vm::main_module;
std::stack<model::Object*> Vm::op_stack{};
std::vector<std::shared_ptr<CallFrame>> Vm::call_stack{};
model::Object Vm::small_int_pool[200] {};
bool Vm::running = false;
std::string Vm::file_path;
model::Object* Vm::curr_error {};
dep::HashMap<model::Object*> Vm::std_modules {};


Vm::Vm(const std::string& file_path_) {
    file_path = file_path_;
    DEBUG_OUTPUT("entry builtin functions...");
    // for (dep::BigInt i = 0; i < 199; i+= 1) {
    //     small_int_pool[i.to_unsigned_long_long()] = model::Int{i}
    // }
    entry_builtins();
    entry_std_modules();
}

void Vm::set_main_module(model::Module* src_module) {
    DEBUG_OUTPUT("loading module...");
    // 合法性校验：防止空指针访问
    assert(src_module != nullptr && "Vm::run_module: 传入的src_module不能为nullptr");
    assert(src_module->code != nullptr && "Vm::run_module: 模块的CodeObject未初始化（code为nullptr）");
    // 注册为main module
    main_module = src_module;
    // 创建模块级调用帧（CallFrame）：模块是顶层执行单元，对应一个顶层调用帧
    auto module_call_frame = std::make_shared<CallFrame>(CallFrame{
        src_module->path,                // 调用帧名称与模块名一致（便于调试）

        src_module,
        dep::HashMap<model::Object*>(), // 初始空局部变量表

        0,                               // 程序计数器初始化为0（从第一条指令开始执行）
        src_module->code->code.size(),   // 执行完所有指令后返回的位置（指令池末尾）
        src_module->code,                // 关联当前模块的CodeObject
        {}
    });

    // 将调用帧压入VM的调用栈
    call_stack.emplace_back(module_call_frame);

    // 初始化VM执行状态：标记为"就绪"
    DEBUG_OUTPUT("start running");
    running = true; // 标记VM为运行状态（等待exec触发执行）
    assert(!call_stack.empty() && "Vm::set_main_module: 调用栈为空，无法执行指令");
    auto& module_frame = *call_stack.back(); // 获取当前模块的调用帧（栈顶）
    assert(module_frame.code_object != nullptr && "Vm::set_main_module: 当前调用帧无关联CodeObject");
    exec_curr_code();
}

void Vm::exec_curr_code() {
    // 循环执行当前调用帧下的所有指令
    while (!call_stack.empty() && running) {
        auto& curr_frame = *call_stack.back();
        // 检查当前帧是否执行完毕
        if (curr_frame.pc >= curr_frame.code_object->code.size()) {
            // 非模块帧则弹出，模块帧则退出循环
            if (call_stack.size() > 1) {
                call_stack.pop_back();
            } else {
                break;
            }
            continue;
        }

        // 执行当前指令
        const Instruction& curr_inst = curr_frame.code_object->code[curr_frame.pc];
        try {
            execute_instruction(curr_inst);
        } catch (NativeFuncError& e) {
            instruction_throw(e.name, e.msg);
        }
        DEBUG_OUTPUT("curr inst is "+opcode_to_string(curr_inst.opc));
        DEBUG_OUTPUT("current stack top : " + (op_stack.empty() ? "[Nothing]" : op_stack.top()->debug_string()));

        if (curr_inst.opc != Opcode::JUMP && curr_inst.opc != Opcode::JUMP_IF_FALSE &&
            curr_inst.opc != Opcode::RET && curr_inst.opc != Opcode::JUMP_IF_FINISH_HANDLE_ERROR) {
            curr_frame.pc++;
        }
    }

    DEBUG_OUTPUT("call stack length: " + std::to_string(call_stack.size()));
}

CallFrame* Vm::fetch_curr_call_frame() {
    if ( !call_stack.empty() ) {
        return call_stack.back().get();
    }
    assert(false);
}

model::Object* Vm::fetch_one_from_stack_top() {
    const auto stack_top = op_stack.empty() ? nullptr : op_stack.top();
    if (stack_top) op_stack.pop();
    return stack_top;
}

void Vm::set_and_exec_curr_code(const model::CodeObject* code_object) {
    DEBUG_OUTPUT("execute_instruction set_and_exec_curr_code (覆盖模式)...");
    DEBUG_OUTPUT("call stack length: " + std::to_string(call_stack.size()));

    // 合法性校验
    assert(code_object != nullptr && "Vm::set_and_exec_curr_code: 传入的 code_object 不能为 nullptr");
    assert(!call_stack.empty() && "Vm::set_and_exec_curr_code: 调用栈为空，需先通过 set_main_module() 加载模块");

    // 获取全局模块级调用帧（REPL 共享同一个帧）
    auto& curr_frame = *call_stack.back();

    // ========== 覆盖：常量池 ==========
    curr_frame.code_object->consts = code_object->consts;

    // ========== 覆盖：名称表 ==========
    curr_frame.code_object->names = code_object->names; // 覆盖 CodeObject 的 names

    // ========== 覆盖：指令 ==========
    curr_frame.code_object->code = code_object->code;
    DEBUG_OUTPUT("set_and_exec_curr_code: 追加指令 ");

    // ========== 执行指令 ==========
    curr_frame.pc = 0;
    exec_curr_code();
    DEBUG_OUTPUT("set_and_exec_curr_code: 执行新指令完成");
}

void Vm::execute_instruction(const Instruction& instruction) {
    switch (instruction.opc) {
        case Opcode::OP_ADD:          exec_ADD(instruction);          break;
        case Opcode::OP_SUB:          exec_SUB(instruction);          break;
        case Opcode::OP_MUL:          exec_MUL(instruction);          break;
        case Opcode::OP_DIV:          exec_DIV(instruction);          break;
        case Opcode::OP_MOD:          exec_MOD(instruction);          break;
        case Opcode::OP_POW:          exec_POW(instruction);          break;
        case Opcode::OP_NEG:          exec_NEG(instruction);          break;
        case Opcode::OP_EQ:           exec_EQ(instruction);           break;
        case Opcode::OP_GT:           exec_GT(instruction);           break;
        case Opcode::OP_LT:           exec_LT(instruction);           break;
        case Opcode::OP_GE:           exec_GE(instruction);           break;
        case Opcode::OP_LE:           exec_LE(instruction);           break;
        case Opcode::OP_NE:           exec_NE(instruction);           break;
        case Opcode::OP_AND:          exec_AND(instruction);          break;
        case Opcode::OP_NOT:          exec_NOT(instruction);          break;
        case Opcode::OP_OR:           exec_OR(instruction);           break;
        case Opcode::OP_IS:           exec_IS(instruction);           break;
        case Opcode::OP_IN:           exec_IN(instruction);           break;
        case Opcode::MAKE_LIST:       exec_MAKE_LIST(instruction);    break;
        case Opcode::MAKE_DICT:       exec_MAKE_DICT(instruction);    break;

        case Opcode::CALL:            exec_CALL(instruction);          break;
        case Opcode::RET:             exec_RET(instruction);           break;
        case Opcode::CALL_METHOD:     exec_CALL_METHOD(instruction);   break;
        case Opcode::GET_ATTR:        exec_GET_ATTR(instruction);      break;
        case Opcode::SET_ATTR:        exec_SET_ATTR(instruction);      break;
        case Opcode::GET_ITEM:        exec_GET_ITEM(instruction);      break;
        case Opcode::SET_ITEM:        exec_SET_ITEM(instruction);      break;
        case Opcode::LOAD_VAR:        exec_LOAD_VAR(instruction);      break;
        case Opcode::LOAD_CONST:      exec_LOAD_CONST(instruction);    break;
        case Opcode::SET_GLOBAL:      exec_SET_GLOBAL(instruction);    break;
        case Opcode::SET_LOCAL:       exec_SET_LOCAL(instruction);     break;

        case Opcode::ENTER_TRY:       exec_ENTER_TRY(instruction);     break;
        case Opcode::MARK_HANDLE_ERROR: exec_MARK_HANDLE_ERROR(instruction); break;
        case Opcode::JUMP_IF_FINISH_HANDLE_ERROR:  exec_JUMP_IF_FINISH_HANDLE_ERROR(instruction);    break;

        case Opcode::IMPORT:          exec_IMPORT(instruction);        break;
        case Opcode::LOAD_ERROR:      exec_LOAD_ERROR(instruction);    break;
        case Opcode::SET_NONLOCAL:    exec_SET_NONLOCAL(instruction);  break;
        case Opcode::JUMP:            exec_JUMP(instruction);          break;
        case Opcode::JUMP_IF_FALSE:   exec_JUMP_IF_FALSE(instruction); break;
        case Opcode::THROW:           exec_THROW(instruction);         break;
        case Opcode::IS_CHILD:        exec_IS_CHILD(instruction);      break;
        case Opcode::CREATE_OBJECT:   exec_CREATE_OBJECT(instruction); break;
        case Opcode::STOP:            exec_STOP(instruction);          break;
        default:                      assert(false && "execute_instruction: 未知 opcode");
    }
}

std::string Vm::obj_to_str(model::Object* for_cast_obj) {
    DEBUG_OUTPUT("obj to str");
    try {
        call_method(for_cast_obj, "__str__", model::create_list({}));
    } catch (NativeFuncError& e) {
        call_method(for_cast_obj, "__dstr__", model::create_list({}));
    }
    auto res = fetch_one_from_stack_top();
    std::string val = model::cast_to_str(res)->val;
    return val;
}


std::string Vm::obj_to_debug_str(model::Object* for_cast_obj) {
    DEBUG_OUTPUT("obj to debug str");
    try {
       call_method(for_cast_obj, "__dstr__", model::create_list({}));
    } catch (NativeFuncError& e) {
       call_method(for_cast_obj, "__str__", model::create_list({}));
    }
    auto res = fetch_one_from_stack_top();
    std::string val = model::cast_to_str(res)->val;
    return val;
}

} // namespace kiz