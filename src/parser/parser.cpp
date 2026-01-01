/**
 * @file parser.cpp
 * @brief 语法分析器（Parser）核心实现
 * 从Token列表生成AST（适配函数定义新语法：fn x() end）
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "parser.hpp"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <iostream>
#include <memory>
#include <vector>

#include "kiz.hpp"
#include "repl/color.hpp"

namespace kiz {

Token Parser::skip_token(const std::string& want_skip) {
    DEBUG_OUTPUT("skipping token: index " + std::to_string(curr_tok_idx_));

    // 边界检查
    if (curr_tok_idx_ >= tokens_.size()) {
        assert("skip_token: 索引越界");
    }

    const Token& curr_tok = tokens_[curr_tok_idx_];
    // 无目标文本：直接跳过当前 Token
    if (want_skip.empty()) {
        ++curr_tok_idx_;
        return curr_tok;
    }

    // 优先按文本匹配
    if (curr_tok.text == want_skip) {
        ++curr_tok_idx_;
        return curr_tok;
    }

    // 严格报错
    assert(false);
}

// curr_token实现
Token Parser::curr_token() const {
    if (curr_tok_idx_ < tokens_.size()) {
        return tokens_.at(curr_tok_idx_);
    }
    return Token{TokenType::EndOfFile, "", 1, 1};
}

// skip_end_of_ln实现
void Parser::skip_end_of_ln() {
    DEBUG_OUTPUT("skipping end of line...");
    const Token curr_tok = curr_token();
    // 支持分号或换行作为语句结束符
    if (curr_tok.type == TokenType::Semicolon) {
        skip_token(";");
        return;
    }
    if (curr_tok.type == TokenType::EndOfLine) {
        skip_token("\n");
        return;
    }
    // 到达文件末尾也视为合法结束
    if (curr_tok.type == TokenType::EndOfFile) {
        return;
    }
    DEBUG_OUTPUT("curr_tok: " + curr_tok.text);
    err::error_reporter(file_path, curr_tok.pos, "SyntaxError", "Invalid statement terminator");
}

// skip_start_of_block实现 处理函数体前置换行
void Parser::skip_start_of_block() {
    DEBUG_OUTPUT("skipping start of block...");
    const Token curr_tok = curr_token();
    // if (curr_tok.type == TokenType::Colon) {
    //     skip_token(":");
    //     return;
    // }
    while (curr_tok.type == TokenType::EndOfLine) {
        skip_token("\n");
        return;
    }
}

// parse_program实现（解析整个程序
std::unique_ptr<BlockStmt> Parser::parse(const std::vector<Token>& tokens) {
    tokens_ = tokens;
    curr_tok_idx_ = 0;
    DEBUG_OUTPUT("parsing...");
    std::vector<std::unique_ptr<Statement>> program_stmts;

    // 打印 Token 序列（保留调试逻辑）
    DEBUG_OUTPUT("=== 所有 Token 序列 ===");
    for (size_t i = 0; i < tokens.size(); i++) {
        const auto& tok = tokens[i];
        DEBUG_OUTPUT(
            "Token[" + std::to_string(i) + "]: type=" + std::to_string(static_cast<int>(tok.type))
            + ", text='" + tok.text + "', line=" + std::to_string(tok.pos.lno_start)
        );
    }
    DEBUG_OUTPUT("=== Token 序列结束 ===");

    // 全局块解析：直到 EOF
    while (curr_token().type != TokenType::EndOfFile) {
        // 跳过前置换行（仅清理，不处理语句）
        while(curr_token().type == TokenType::EndOfLine) {
            skip_token(); // 直接跳过换行
        }
        if (auto stmt = parse_stmt(); stmt != nullptr) {
            program_stmts.push_back(std::move(stmt));
        }
    }

    DEBUG_OUTPUT("end parsing");
    constexpr err::PositionInfo pos = {1, 1, 1, 1};
    return std::make_unique<BlockStmt>(pos, std::move(program_stmts));
}

} // namespace kiz