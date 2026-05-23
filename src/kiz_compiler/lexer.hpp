#pragma once

namespace kizc {

enum class TokenKind: uint8_t {
    OpAdd, OpSub, OpMul, OpDiv, OpMod, OpEq,
    OpNe, OpGt, OpLt, OpLe, OpGe,
    KwAnd, KwOr, KwNot, 
    KwIf, KwElif, KwElse,
    KwWhile, KwLoop, 
    KwBreak, KwReturn, KwNext,
    KwType, KwWith, KwImpl, KwAbstract,
    KwMove, KwRef, KwMutRef, KwClone,
    kwLet, KwPub, KwUse, KwMod,
    KwFun, KwTry, KwWhen,
    At, LParen, RParen, LBrace, RBrace, LBracket, RBracket,
    Comma, Semi, Colon, Dot, FatArrow, ThinArrow, Assign,
    Ident, String, Int, Decimal, Eof, Invaild
};

struct Token {
    TokenKind tokenkind;
    Str text;
    Span span;
};

class Lexer {
    Str text_;
    uint32_t pos_;
    uint32_t lineno_;
    uint8_t col_;
    Token next_cache_;
public:
    Lexer(Str s)
        : text_(s), pos_(0), lineno_(1), col_(1) {}
    /// next
    auto next() -> Token;
    auto peek() -> Token;
}

}