#pragma once

struct ComptimeError {
    Str msg;
    ComptimeErrorKind kind;
    Span span;
};