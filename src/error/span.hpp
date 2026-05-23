#pragma once

struct Span {
    Str filepath;
    uint32_t lineno_start;
    uint32_t lineno_end;
    uint16_t col_start;
    uint16_t col_end;
};