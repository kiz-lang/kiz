#pragma once

namespace kizc {

enum class TokenKind: uint8_t {
    OpAdd, OpSub, OpMul, OpDiv, OpMod,
    OpEq, OpNe, OpGt, OpLt, OpLe, OpGe,
    OpBitAnd, OpBitOr, OpBitXor, OpNotBit, OpBitLShift, OpBitRShift,

    OpLogicAnd, OpLogicOr, OpLogicNot, 
    KwIf, KwElif, KwElse,
    KwWhile, KwLoop, KwFor, KwIn,
    KwBreak, KwReturn, KwContinue,
    KwType, KwError, KwExternal,
    KwImpl, KwAbstract,
    KwMove, KwRef, KwMutRef, KwClone,
    KwLet, KwFun, KwGlobal, KwFinal,
    KwWhen, KwBindto,
    KwTry, KwOn, KwEffect, KwResume,

    At, Question, Hash,
    LParen, RParen, 
    LBrace, RBrace, 
    LBracket, RBracket,
    Comma, Semi, Colon, Dot, 
    FatArrow, ThinArrow, 
    Assign, Pipe, Ellipsis,

    Ident, String, Int, Decimal, 

    Eof, Invalid,
};

struct Token {
    Span span;
    Str text;
    TokenKind tokenkind;
};

class Lexer {
    FileMetaData file_;
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