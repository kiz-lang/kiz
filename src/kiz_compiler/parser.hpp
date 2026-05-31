#pragma once

namespace kizc {

class Parser {
    Vec<Str> import_paths_;
    Vec<SymbolTable> define_stack_;

    Vec<FunctionSymbol> global_function_symbols_;
    Vec<AbstractSymbol> global_abstract_symbols_;
    Vec<TypeSymbol> global_type_symbols_;

    Lexer& lexer_;
    Codegen& codegen_;
    IRModule& irmodule_;

    /// parser
    auto parse_expr() -> ComptimeError;
    auto parse_stmt() -> ComptimeError;
    auto parse_if() -> ComptimeError;
    auto parse_loop() -> ComptimeError;
    auto parse_fn() -> ComptimeError;
    auto parse_typedef() -> ComptimeError;
    auto parse_abstract() -> ComptimeError;
    auto parse_handler() -> ComptimeError;

    /// sema
    auto lookup_var(Str name) -> uint32_t;
    auto lookup_fn(Str name) -> uint32_t;
    auto lookup_type(Str name) -> uint32_t;
    auto lookup_abstract(Str name) -> uint32_t;
    auto lookup_field(uint32_t tyid) -> uint32_t;
    auto lookup_method(uint32_t tyid) -> uint32_t;

    auto check_call(uint32_t fn, Vec<uint32_t> argv) -> bool;
    auto analyze_binary(Type l, Type r) -> Result<Type, ComptimeError>;
    auto analyze_unary(Type l, Type r) -> Result<Type, ParserError>;

public:
    Parser(Vec<Str> import_paths, Lexer& l, Codegen& cg)
    : import_paths_(import_paths), lexer_(l), codegen_(cg), define_stack_({}), irmodule_({}) {}
    
    auto parse(Str txt) -> Result<IRModule, ComptimeError>;
}

}