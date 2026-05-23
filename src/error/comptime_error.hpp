#pragma once

#define $Err(x) x,
enum class ComptimeErrorKind: uint8_t {
    #include "comptime_error_kind.def"
};
#undef $Err

#define $Err(x) case x: return Str(#x);
inline auto comptime_error_kind_to_string(ComptimeErrorKind k) -> Str {
    switch(k) {
    #include "comptime_error_kind.def"
    }
}
#undef $Err

struct ComptimeError {
    Span span;
    Str msg;
    ComptimeErrorKind kind;
};