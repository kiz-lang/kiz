#include "../../src/models/models.hpp"
#include "include/builtin_functions.hpp"

namespace model {

// Bool.__call__
Object* bool_call(Object* self, const List* args) {
    const auto a = builtin::get_one_arg(args);
    return load_bool(
        kiz::Vm::is_true(a)
    );
}

Object* bool_str(Object* self, const List* args) {
    const auto s = dynamic_cast<Bool*>(self);
    return new String(s->val ? "True" : "False");
}


// Bool.__eq__ 布尔值相等判断：self == args[0]（仅支持Bool与Bool比较）
Object* bool_eq(Object* self, const List* args) {
    kiz::Vm::assert_argc(1, args);
    
    auto self_bool = dynamic_cast<Bool*>(self);
    auto another_bool = dynamic_cast<Bool*>(args->val[0]);
    if (!another_bool)
        throw NativeFuncError("TypeError", "Bool.eq only supports Bool type argument");
    
    return load_bool(self_bool->val == another_bool->val);
}

// Bool.__hash__
Object* bool_hash(Object* self, const List* args) {
    auto self_bool = dynamic_cast<Bool*>(self);
    if (self_bool->val == true) {
        return kiz::Vm::small_int_pool[1];
    }
    return kiz::Vm::small_int_pool[0];
}


}  // namespace model