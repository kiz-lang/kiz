#pragma once

struct Object {
    Ptr<Value> fields;
    uint32_t field_count
    uint32_t tyid;
    uint32_t refcount;
    bool is_moved;
};