#include "models.hpp"

namespace model {

// Bool.__call__
model::Object* str_bool(model::Object* self, const model::List* args) {
    const auto a = get_one_arg();
    return new model::Bool(
        kiz::Vm::check_is_true(a)
    );
}

// Bool.__eq__ 布尔值相等判断：self == args[0]（仅支持Bool与Bool比较）
model::Object* bool_eq(model::Object* self, const model::List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (bool_eq)");
    assert(args->val.size() == 1 && "function Bool.eq need 1 arg");
    
    auto self_bool = dynamic_cast<Bool*>(self);
    auto another_bool = dynamic_cast<Bool*>(args->val[0]);
    assert(another_bool != nullptr && "Bool.eq only supports Bool type argument");
    
    return new Bool(self_bool->val == another_bool->val);
};

}  // namespace model