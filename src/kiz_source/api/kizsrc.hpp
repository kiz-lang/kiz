#pragma once

namespace kizsrc {
class SourceManager;
}

namespace kizsrc::api {

enum class DiagLevel : uint8_t {
    Warn, Error, Fatal
};

auto print_error_head(Span span, Str def_name) -> void;

auto print_span(
    SourceManager& manager,
    Span span
) -> void;

[[noreturn]]
auto print_main_error(
    SourceManager& manager,
    Span span,
    Str msg,
    Str help,
    DiagLevel level
) -> void;

}