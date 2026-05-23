#pragma once

#define $Err(x) x,
enum class RuntimeErrorKind: uint8_t {
    #include "comptime_error_kind.def"
};
#undef $Err

#define $Err(x) case x: return Str(#x);
inline auto runtime_error_kind_to_string(RuntimeErrorKind k) -> Str {
    switch(k) {
    #include "runtime_error_kind.def"
    }
}
#undef $Err

struct RuntimeError {
    Span span;
    Str msg;
    RuntimeErrorKind kind;
};