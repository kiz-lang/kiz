#pragma once

struct Object {
    Vec<Value> fields;
    uint32_t tyid;
    bool is_moved;
    uint32_t refcount;
};