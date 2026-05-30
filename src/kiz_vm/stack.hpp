#pragma once


struct Segment {
    Ptr<Value> mem_start;
    Ptr<Value> mem_end;
    uint32_t bind_handler;
    uint32_t pred;
    uint32_t succ;
    uint32_t pc;
};

struct Stack {
    Vec<Value> stack_mem_;
    /*
    MemoryPool<Value> stack_mem_pool_;
    Vec<Segment> segment_pool_;
    uint32_t current_segement_;
    uint32_t current_effect_id;

    Segment stack_;
    Segment tmp_continuation_;
    */
};