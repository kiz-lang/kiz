/**
 * @file repl.cpp
 * @brief 交互式终端（Read Evaluate Print Loop，简称 REPL）核心实现
 *
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "repl/repl.hpp"

#include "lexer.hpp"
#include "vm.hpp"
#include "ir_gen.hpp"
#include "parser.hpp"
#include "kiz.hpp"
#include "repl/color.hpp"
#include "util/src_manager.hpp"

namespace ui {

const std::string Repl::file_path = "<shell#>";

Repl::Repl(): is_running_(true) , vm_(file_path) {
    std::cout << "This is the kiz REPL " << KIZ_VERSION << "\n" << std::endl;
}

std::string Repl::read(const std::string& prompt) {
    std::string result;
    std::cout << Color::BRIGHT_MAGENTA << prompt << Color::RESET;
    std::cout.flush();
    std::getline(std::cin, result);
    return result;
}

void Repl::loop() {
    DEBUG_OUTPUT("start repl loop");
    while (is_running_) {
        try {
            auto code = read(">>>");
            auto old_code_it = err::opened_files.find(file_path);
            if (old_code_it != err::opened_files.end()) {
                err::opened_files[file_path] = old_code_it->second + "\n" + code;
            } else {
                err::opened_files[file_path] = code;
            }

            add_to_history(code);
            eval_and_print(code);
        } catch (KizStopRunningSignal& e) {
            // todo
        } catch (...) {
            // todo
        }
    }
}

void Repl::eval_and_print(const std::string& cmd) {
    DEBUG_OUTPUT("repl eval_and_print...");
    bool should_print = false;
    kiz::Lexer lexer(file_path);
    kiz::Parser parser(file_path);
    kiz::IRGenerator ir_gen(file_path);

    const auto tokens = lexer.tokenize(cmd, get_history().size());
    auto ast = parser.parse(tokens);
    if (!ast->statements.empty() and
        dynamic_cast<kiz::ExprStmt*>(ast->statements.back().get())
    )   should_print = true;

    const auto ir = ir_gen.gen(std::move(ast));
    if (cmd_history_.size() < 2) {
        const auto module = kiz::IRGenerator::gen_mod(file_path, ir);
        vm_.set_main_module(module);
    } else {
        assert(ir != nullptr && "No ir for run" );
        vm_.set_curr_code(ir);
    }

    DEBUG_OUTPUT("repl print");
    auto stack_top = vm_.get_stack_top();
    if (stack_top != nullptr) {
        if (not dynamic_cast<model::Nil*>(stack_top) and should_print) {
            std::cout << stack_top->to_string() << std::endl;
        }
    }
}

} // namespace repl