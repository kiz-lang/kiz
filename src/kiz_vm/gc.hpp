#pragma once


enum class GCPhase : uint8_t {
    Idle,
    Mark, 
    Sweep,
    Compact
};

enum class ObjMark : uint8_t {
    White, 
    Gray,
    Black
};

class GarbageCollection {
    Object& root_object_;
    GCPhase phase_;
    Vec<Ptr<Object>> work_list_;

    void mark_reachable();
public:
    explicit GarbageCollection(Object& root) 
        : root_object_(root), phase_(GCPhase::Idle) {}

    auto collect() -> void;
    auto mark_object(Ptr<Object> obj) -> void;
    auto sweep() -> void;
    
};