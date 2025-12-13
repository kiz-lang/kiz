#pragma once
#include <unordered_set>

#include "models.hpp"

namespace builtin {

inline model::Object* get_one_arg(const model::List* args) {
    if (!args->val.empty()) {
        return args->val[0];
    }
    assert(false && "函数参数不足一个");
}

inline model::Object* check_based_object_inner(
    model::Object* src_obj,
    model::Object* for_check_obj,
    std::unordered_set<model::Object*>& visited
) {
    if (src_obj == nullptr) return nullptr;
    // 闭环检测
    if (visited.contains(src_obj)) return new model::Bool(false);
    visited.insert(src_obj);

    // 查找__parent__属性
    const auto it = src_obj->attrs.find("__parent__");
    if (it == nullptr) {
        return new model::Bool(false);
    }
    // 找到目标返回true，否则递归检查父对象
    if (it->value == for_check_obj) return new model::Bool(true);
    return check_based_object_inner(it->value, for_check_obj, visited);
}

// 对外接口
inline model::Object* check_based_object(model::Object* src_obj, model::Object* for_check_obj) {
    std::unordered_set<model::Object*> visited; // 每次调用新建集合
    return check_based_object_inner(src_obj, for_check_obj, visited);
}

// 内置函数
model::Object* print(model::Object* self, const model::List* args);
model::Object* input(model::Object* self, const model::List* args);
model::Object* isinstance(model::Object* self, const model::List* args);
model::Object* help(model::Object* self, const model::List* args);
model::Object* breakpointer(model::Object* self, const model::List* args);
model::Object* range(model::Object* self, const model::List* args);
model::Object* cmd(model::Object* self, const model::List* args);
model::Object* now(model::Object* self, const model::List* args);
model::Object* setattr(model::Object* self, const model::List* args);
model::Object* getattr(model::Object* self, const model::List* args);
model::Object* delattr(model::Object* self, const model::List* args);
model::Object* getrefc(model::Object* self, const model::List* args);
model::Object* copy(model::Object* self, const model::List* args);
model::Object* create(model::Object* self, const model::List* args);
model::Object* typeofobj(model::Object* self, const model::List* args);


}