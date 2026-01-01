/**
 * @file vm.cpp
 * @brief 虚拟机（VM）核心实现
 * 用于运行IR
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "vm.hpp"

#include "models.hpp"
#include "opcode.hpp"

#include <algorithm>
#include <cassert>

#include "kiz.hpp"

#include "../libs/builtins/include/builtin_methods.hpp"
#include "../libs/builtins/include/builtin_functions.hpp"

namespace kiz {

dep::HashMap<model::Object*> Vm::builtins{};
dep::HashMap<model::Module*> Vm::loaded_modules{};
model::Module* Vm::main_module;
std::stack<model::Object*> Vm::op_stack{};
std::vector<std::shared_ptr<CallFrame>> Vm::call_stack{};
bool Vm::running = false;
std::string Vm::file_path;
model::Error* Vm::curr_error {};

std::pair<std::string, std::string> get_err_name_and_msg(const model::Object* err_obj) {
    assert(err_obj != nullptr);
    auto err_name_it = err_obj->attrs.find("__name__");
    auto err_msg_it = err_obj->attrs.find("__msg__");
    assert(err_name_it != nullptr);
    assert(err_msg_it != nullptr);
    auto err_name = err_name_it->value->to_string();
    auto err_msg = err_msg_it->value->to_string();
    return {err_name, err_msg};
}

Vm::Vm(const std::string& file_path_) {
    file_path = file_path_;
    DEBUG_OUTPUT("registering builtin functions...");
    builtins.insert("print", new model::CppFunction(builtin::print));
    builtins.insert("input", new model::CppFunction(builtin::input));
    builtins.insert("isinstance", new model::CppFunction(builtin::isinstance));
    builtins.insert("create", new model::CppFunction(builtin::create));
    builtins.insert("now", new model::CppFunction(builtin::now));
    builtins.insert("get_refc", new model::CppFunction(builtin::get_refc));
    builtins.insert("breakpoint", new model::CppFunction(builtin::breakpoint));

    DEBUG_OUTPUT("registering builtin objects...");
    builtins.insert("obj", model::based_obj);

    model::based_bool->attrs.insert("__parent__", model::based_obj);
    model::based_int->attrs.insert("__parent__", model::based_obj);
    model::based_nil->attrs.insert("__parent__", model::based_obj);
    model::based_rational->attrs.insert("__parent__", model::based_obj);
    model::based_function->attrs.insert("__parent__", model::based_obj);
    model::based_dict->attrs.insert("__parent__", model::based_obj);
    model::based_list->attrs.insert("__parent__", model::based_obj);
    model::based_str->attrs.insert("__parent__", model::based_obj);

    DEBUG_OUTPUT("registering magic methods...");

    // Object 基类 __eq__
    model::based_obj->attrs.insert("__eq__", new model::CppFunction([](const model::Object* self, const model::List* args) -> model::Object* {
        const auto other_obj = builtin::get_one_arg(args);
        return new model::Bool(self == other_obj);
    }));
    model::based_obj->attrs.insert("__str__", new model::CppFunction([](const model::Object* self, const model::List* args) -> model::Object* {
        return new model::String(self->to_string());
    }));

    model::based_obj->attrs.insert("__getitem__", new model::CppFunction([](const model::Object* self, const model::List* args) -> model::Object* {
        auto attr = builtin::get_one_arg(args);
        auto attr_str = dynamic_cast<model::String*>(attr);
        assert(attr_str != nullptr);
        return get_attr(self, attr_str->val);
    }));

    model::based_obj->attrs.insert("__setitem__", new model::CppFunction([](model::Object* self, model::List* args) -> model::Object* {
        assert(args->val.size() == 2);
        auto attr = args->val[0];
        auto attr_str = dynamic_cast<model::String*>(attr);
        assert(attr_str != nullptr);
        self->attrs.insert(attr_str->val, args->val[1]);
        return self;
    }));

    // Bool 类型魔法方法
    model::based_bool->attrs.insert("__eq__", new model::CppFunction(model::bool_eq));
    model::based_bool->attrs.insert("__call__", new model::CppFunction(model::bool_call));

    // Nil 类型魔法方法
    model::based_nil->attrs.insert("__eq__", new model::CppFunction(model::nil_eq));

    // Int 类型魔法方法
    model::based_int->attrs.insert("__add__", new model::CppFunction(model::int_add));
    model::based_int->attrs.insert("__sub__", new model::CppFunction(model::int_sub));
    model::based_int->attrs.insert("__mul__", new model::CppFunction(model::int_mul));
    model::based_int->attrs.insert("__div__", new model::CppFunction(model::int_div));
    model::based_int->attrs.insert("__mod__", new model::CppFunction(model::int_mod));
    model::based_int->attrs.insert("__pow__", new model::CppFunction(model::int_pow));
    model::based_int->attrs.insert("__gt__", new model::CppFunction(model::int_gt));
    model::based_int->attrs.insert("__lt__", new model::CppFunction(model::int_lt));
    model::based_int->attrs.insert("__eq__", new model::CppFunction(model::int_eq));
    model::based_int->attrs.insert("__call__", new model::CppFunction(model::int_call));
    model::based_int->attrs.insert("__bool__", new model::CppFunction(model::int_bool));

    // Rational 类型魔法方法
    model::based_rational->attrs.insert("__add__", new model::CppFunction(model::rational_add));
    model::based_rational->attrs.insert("__sub__", new model::CppFunction(model::rational_sub));
    model::based_rational->attrs.insert("__mul__", new model::CppFunction(model::rational_mul));
    model::based_rational->attrs.insert("__div__", new model::CppFunction(model::rational_div));
    model::based_rational->attrs.insert("__gt__", new model::CppFunction(model::rational_gt));
    model::based_rational->attrs.insert("__lt__", new model::CppFunction(model::rational_lt));
    model::based_rational->attrs.insert("__eq__", new model::CppFunction(model::rational_eq));

    // Dictionary 类型魔法方法
    model::based_dict->attrs.insert("__add__", new model::CppFunction(model::dict_add));
    model::based_dict->attrs.insert("__contains__", new model::CppFunction(model::dict_contains));

    // List 类型魔法方法
    model::based_list->attrs.insert("__add__", new model::CppFunction(model::list_add));
    model::based_list->attrs.insert("__mul__", new model::CppFunction(model::list_mul));
    model::based_list->attrs.insert("__eq__", new model::CppFunction(model::list_eq));
    model::based_list->attrs.insert("__call__", new model::CppFunction(model::list_call));
    model::based_list->attrs.insert("__bool__", new model::CppFunction(model::list_bool));
    model::based_list->attrs.insert("__next__", new model::CppFunction(model::list_next));
    model::based_list->attrs.insert("append", new model::CppFunction(model::list_append));
    model::based_list->attrs.insert("contains", new model::CppFunction(model::list_contains));
    
    // String 类型魔法方法
    model::based_str->attrs.insert("__add__", new model::CppFunction(model::str_add));
    model::based_str->attrs.insert("__mul__", new model::CppFunction(model::str_mul));
    model::based_str->attrs.insert("__eq__", new model::CppFunction(model::str_eq));
    model::based_str->attrs.insert("__call__", new model::CppFunction(model::str_call));
    model::based_str->attrs.insert("__bool__", new model::CppFunction(model::str_bool));
    model::based_str->attrs.insert("contains", new model::CppFunction(model::str_contains));

    model::based_error->attrs.insert("__call__", new model::CppFunction([](model::Object* self, model::List* args) -> model::Object* {   assert( args->val.size() == 2);
        auto err_name = args->val[0];
        auto err_msg = args->val[1];

        auto err = new model::Error();
        err->attrs.insert("__name__", err_name);
        err->attrs.insert("__msg__", err_msg);
        return err;
    }));

    builtins.insert("int", model::based_int);
    builtins.insert("bool", model::based_bool);
    builtins.insert("rational", model::based_rational);
    builtins.insert("list", model::based_list);
    builtins.insert("dict", model::based_dict);
    builtins.insert("str", model::based_str);
    builtins.insert("function", model::based_function);
    builtins.insert("nil", model::based_nil);
    builtins.insert("Error", model::based_error);
    DEBUG_OUTPUT("current builtins: " + builtins.to_string());
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
        src_module->name,                // 调用帧名称与模块名一致（便于调试）

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
        execute_instruction(curr_inst);
        DEBUG_OUTPUT("curr inst is "+opcode_to_string(curr_inst.opc));
        DEBUG_OUTPUT("current stack top : " + (op_stack.empty() ? " " : op_stack.top()->to_string()));

        // 修正PC自增条件：仅非跳转/非RET指令自增
        if (curr_inst.opc != Opcode::JUMP && curr_inst.opc != Opcode::JUMP_IF_FALSE && curr_inst.opc != Opcode::RET) {
            curr_frame.pc++;
        }
    }

    DEBUG_OUTPUT("call stack length: " + std::to_string(call_stack.size()));
}

model::Object* Vm::get_return_val() {
    assert(!op_stack.empty());
    return op_stack.top();
}

CallFrame* Vm::fetch_curr_call_frame() {
    if ( !call_stack.empty() ) {
        return call_stack.back().get();
    }
    assert(false);
}

model::Object* Vm::fetch_one_from_stack_top() {
    if ( !op_stack.empty() ) {
        return op_stack.top();
    }
    assert(false);
}

void Vm::set_curr_code(const model::CodeObject* code_object) {
    DEBUG_OUTPUT("execute_instruction set_curr_code (覆盖模式)...");
    DEBUG_OUTPUT("call stack length: " + std::to_string(call_stack.size()));

    // 合法性校验
    assert(code_object != nullptr && "Vm::set_curr_code: 传入的 code_object 不能为 nullptr");
    assert(!call_stack.empty() && "Vm::set_curr_code: 调用栈为空，需先通过 set_main_module() 加载模块");

    // 获取全局模块级调用帧（REPL 共享同一个帧）
    auto& curr_frame = *call_stack.back();

    // ========== 覆盖：常量池 ==========
    curr_frame.code_object->consts = code_object->consts;

    // ========== 覆盖：名称表 ==========
    curr_frame.code_object->names = code_object->names; // 覆盖 CodeObject 的 names

    // ========== 覆盖：指令 ==========
    curr_frame.code_object->code = code_object->code;
    DEBUG_OUTPUT("set_curr_code: 追加指令 ");

    // ========== 执行指令 ==========
    curr_frame.pc = 0;
    exec_curr_code();
    DEBUG_OUTPUT("set_curr_code: 执行新指令完成");
}

void Vm::load_required_modules(const dep::HashMap<model::Module*>& modules) {
    loaded_modules = modules;
}

model::Object* Vm::get_stack_top() {
    auto stack_top = op_stack.empty() ? nullptr : op_stack.top();
    return stack_top;
}

std::vector<std::pair<std::string, err::PositionInfo>> Vm::gen_positions() {
    size_t i = 0;
    std::vector<std::pair<std::string, err::PositionInfo>> positions;
    std::string path;
    for (const auto& frame: call_stack) {
        if (const auto m = dynamic_cast<model::Module*>(frame->owner)) {
            path = m->name;
        }
        err::PositionInfo pos {};
        if (i == call_stack.size() - 1) {
            pos = frame->code_object->code.at(frame->pc).pos;
        } else {
            pos = frame->code_object->code.at(frame->pc - 1).pos;
        }
        positions.emplace_back(path, pos);
        ++i;
    }
    return positions;
}

void Vm::native_fn_throw(const std::string& name, const std::string& content) {
    const auto err_name = new model::String(name);
    const auto err_msg = new model::String(content);

    const auto err_obj = new model::Error();
    err_obj->positions = gen_positions();
    err_obj->attrs.insert("__name__", err_name);
    err_obj->attrs.insert("__msg__", err_msg);
    DEBUG_OUTPUT("err_obj pos size = "+std::to_string(err_obj->positions.size()));;
    throw_error(err_obj);
}

void Vm::throw_error(model::Object* err) {
    const auto error = dynamic_cast<model::Error*>(err);
    assert(error != nullptr);
    if (! curr_error) {
        DEBUG_OUTPUT("setting curr err");
        curr_error = error;
        curr_error->positions = (error->positions.size() == 0)
            ? gen_positions()
            : error->positions;
        DEBUG_OUTPUT("curr err: "+curr_error->to_string());
        DEBUG_OUTPUT("curr err pos size: "+std::to_string(curr_error->positions.size()));
    }

    size_t frames_to_pop = 0;
    CallFrame* target_frame = nullptr;
    size_t catch_pc = 0;

    // 逆序遍历调用栈，寻找最近的 try 块（且当前不在该 catch 块内）
    for (auto frame_it = call_stack.rbegin(); frame_it != call_stack.rend(); ++frame_it) {
        CallFrame* frame = (*frame_it).get();
        if (!frame->try_blocks.empty()) {
            target_frame = frame;
            catch_pc = frame->try_blocks.back().catch_start;
            // 关键判断：如果当前 PC 已经在 catch 块内（>= catch_pc），则不重复捕获
            if (frame->pc >= catch_pc) {
                target_frame = nullptr; // 取消当前 try 块的捕获
                frames_to_pop++;        // 继续向上查找更外层的 try 块
                continue;
            }
            break;
        }
        frames_to_pop++;
    }

    // 如果找到有效的 try 块（当前不在 catch 内）
    if (target_frame) {
        // 弹出多余的栈帧
        for (size_t i = 0; i < frames_to_pop; ++i) {
            call_stack.pop_back();
        }
        // 设置 pc 到 catch 块开始
        target_frame->pc = catch_pc;
        return;
    }

    auto [error_name, error_msg] = get_err_name_and_msg(curr_error);

    // 报错
    std::cout << Color::BRIGHT_RED << "\nTrace Back: " << Color::RESET << std::endl;
    DEBUG_OUTPUT("curr err pos size: "+std::to_string(curr_error->positions.size()));
    for (auto& [_path, _pos]: curr_error->positions ) {
        DEBUG_OUTPUT(_path + " " + std::to_string(_pos.lno_start) + " "
            + std::to_string(_pos.lno_end) + " " + std::to_string(_pos.col_start) + " " + std::to_string(_pos.col_end));
        err::context_printer(_path, _pos);
    }

    DEBUG_OUTPUT(error_name+" "+error_msg);

    // 错误信息（类型加粗红 + 内容白）
    std::cout << Color::BOLD << Color::BRIGHT_RED << error_name
              << Color::RESET << Color::WHITE << " : " << error_msg
              << Color::RESET << std::endl;
    std::cout << std::endl;

    throw KizStopRunningSignal();
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
        case Opcode::OP_AND:          exec_AND(instruction);          break;
        case Opcode::OP_NOT:          exec_NOT(instruction);          break;
        case Opcode::OP_OR:           exec_OR(instruction);           break;
        case Opcode::OP_IS:           exec_IS(instruction);           break;
        case Opcode::OP_IN:           exec_IN(instruction);           break;
        case Opcode::MAKE_LIST:       exec_MAKE_LIST(instruction);    break;

        case Opcode::CALL:            exec_CALL(instruction);          break;
        case Opcode::RET:             exec_RET(instruction);           break;
        case Opcode::CALL_METHOD:     exec_CALL_METHOD(instruction);   break;
        case Opcode::GET_ATTR:        exec_GET_ATTR(instruction);      break;
        case Opcode::SET_ATTR:        exec_SET_ATTR(instruction);      break;
        case Opcode::LOAD_VAR:        exec_LOAD_VAR(instruction);      break;
        case Opcode::LOAD_CONST:      exec_LOAD_CONST(instruction);    break;
        case Opcode::SET_GLOBAL:      exec_SET_GLOBAL(instruction);    break;
        case Opcode::SET_LOCAL:       exec_SET_LOCAL(instruction);     break;
        case Opcode::TRY_START:       exec_TRY_START(instruction);     break;
        case Opcode::TRY_END:         exec_TRY_END(instruction);       break;
        case Opcode::IMPORT:          exec_IMPORT(instruction);        break;
        case Opcode::LOAD_ERROR:      exec_LOAD_ERROR(instruction);    break;
        case Opcode::CLEAN_ERROR:     exec_CLEAN_ERROR(instruction);   break;
        case Opcode::SET_ERROR:       exec_SET_ERROR(instruction);     break;
        case Opcode::SET_NONLOCAL:    exec_SET_NONLOCAL(instruction);  break;
        case Opcode::JUMP:            exec_JUMP(instruction);          break;
        case Opcode::JUMP_IF_FALSE:   exec_JUMP_IF_FALSE(instruction); break;
        case Opcode::THROW:           exec_THROW(instruction);         break;
        case Opcode::POP_TOP:         exec_POP_TOP(instruction);       break;
        case Opcode::SWAP:            exec_SWAP(instruction);          break;
        case Opcode::COPY_TOP:        exec_COPY_TOP(instruction);      break;
        case Opcode::IS_INSTANCE:     exec_IS_INSTANCE(instruction);   break;
        case Opcode::STOP:            exec_STOP(instruction);          break;
        default:                      assert(false && "execute_instruction: 未知 opcode");
    }
}

} // namespace kiz