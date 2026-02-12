#include "vm.hpp"
#include "../models/models.hpp"
#include "opcode/opcode.hpp"

namespace kiz {

// 辅助函数
auto Vm::make_pos_info() -> std::vector<std::pair<std::string, err::PositionInfo>> {
    size_t frame_index = 0;
    std::vector<std::pair<std::string, err::PositionInfo>> positions;
    std::string path;
    for (const auto& frame: call_stack) {
        if (const auto m = dynamic_cast<model::Module*>(frame->owner)) {
            path = m->path;
        }
        err::PositionInfo pos{};
        bool is_last_frame = frame_index == call_stack.size() - 1;
        if (is_last_frame) {
            pos = frame->code_object->code.at(frame->pc).pos;
        } else {
            pos = frame->code_object->code.at(frame->pc - 1).pos;
        }
        positions.emplace_back(path, pos);
        ++frame_index;
    }
    return positions;
}

void Vm::make_list(size_t len) {
    size_t elem_count = len;
    assert(op_stack.size() >= elem_count);

    std::vector<model::Object*> elem_list;
    elem_list.reserve(elem_count);

    for (size_t i = 0; i < elem_count; ++i) {
        model::Object* elem = get_and_pop_stack_top(); // 弹出
        elem_list.push_back(elem);
    }
    std::ranges::reverse(elem_list); // 恢复原序

    auto* list_obj = new model::List(elem_list);      // 内部为每个元素 make_ref
    push_to_stack(list_obj);

    // 释放栈对每个元素的引用（get_and_pop_stack_top 不释放引用）
    for (auto e : elem_list) e->del_ref();
}

void Vm::make_dict(size_t len) {
    size_t elem_count = len;
    const size_t total_elems = elem_count * 2;
    assert(op_stack.size() >= total_elems);

    std::vector<std::pair<dep::BigInt, std::pair<model::Object*, model::Object*>>> elem_list;
    elem_list.reserve(elem_count);

    for (size_t i = 0; i < elem_count; ++i) {
        model::Object* value = get_and_pop_stack_top(); // 弹出 value
        model::Object* key = get_and_pop_stack_top();   // 弹出 key

        // 计算哈希
        call_method(key, "__hash__", {});
        model::Object* hash_obj = get_and_pop_stack_top();
        auto* hashed_int = dynamic_cast<model::Int*>(hash_obj);
        if (!hashed_int) {
            // 异常：释放已弹出的对象和哈希对象
            hash_obj->del_ref();
            key->del_ref();
            value->del_ref();
            throw NativeFuncError("TypeError", "__hash__ must return an integer");
        }
        elem_list.emplace_back(hashed_int->val, std::pair{key, value});
        hash_obj->del_ref();   // 释放哈希结果

        // 释放栈对 key/value 的引用（get_and_pop_stack_top 不释放）
        key->del_ref();
        value->del_ref();
    }

    auto* dict_obj = new model::Dictionary(dep::Dict(elem_list)); // 内部为 key/value make_ref
    push_to_stack(dict_obj);
}
}