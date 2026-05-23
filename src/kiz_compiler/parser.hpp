#pragma once

namespace kizc {

class Parser {
    Lexer& lexer_;
    Codegen& codegen_;
    IRModule& irmodule_;
    Vec<SymbolTable> define_stack_;

    /// parse expr
    auto parse_expr() -> void;

    /// parse stmt
    auto parse_stmt() -> void;
    auto parse_if() -> void;
    auto parse_loop() -> void;
    auto parse_fn() -> void;
    auto parse_typedef() -> void;

    /// sema
    auto lookup_var(Str name) -> Symbol;
    auto lookup_fn(Str name) -> Type;
    auto lookup_type(Str name) -> Type;
    auto define_var(Symbol sym) -> void;
    auto define_fn(Symbol sym) -> void;
    auto define_type(Type sym) -> void;

    auto check_call(Symbol fn, Vec<Symbol> agrv) -> bool;
    auto analyze_binary(Type l, Type r) -> Result<Type, ParserError>;
    auto analyze_unary(Type l, Type r) -> Result<Type, ParserError>;

public:
    Parser(Lexer& l, Codegen& cg)
    : lexer_(l), codegen_(cg), define_stack_({}), irmodule_({}) {}
    
    /// parse
    auto parse(Str txt) -> IRModule;
}

}