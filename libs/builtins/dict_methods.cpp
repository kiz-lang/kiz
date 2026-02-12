#include "../../src/models/models.hpp"
#include "include/builtin_functions.hpp"

namespace model {

dep::BigInt hash_object(Object* key_obj) {
    // hash对象
    kiz::Vm::call_method(key_obj, "__hash__", {});

    const auto result = kiz::Vm::get_stack_top();

    const auto result_int = dynamic_cast<Int*>(result);
    if (!result_int)
        throw NativeFuncError("TypeError", "Object's hash method return a value which type isn't Int");
    dep::BigInt key_hash_val = result_int->val;
    return key_hash_val;
}

// Dictionary.__add__
Object* dict_add(Object* self, const List* args) {
    kiz::Vm::assert_argc(1, args);
    
    auto self_dict = dynamic_cast<Dictionary*>(self);
    assert(self_dict != nullptr);
    
    auto another_dict = dynamic_cast<Dictionary*>(args->val[0]);
    if (! another_dict)
        throw NativeFuncError("TypeError", "Dict.add first argument must be Dict type");

    auto self_dict_to_vec = self_dict->val.to_vector();
    auto another_dict_to_vec = another_dict->val.to_vector();

    self_dict_to_vec.insert(
        self_dict_to_vec.end(),
        another_dict_to_vec.begin(),
        another_dict_to_vec.end()
    );
    
    auto new_dict = new Dictionary(dep::Dict(
        self_dict_to_vec
    ));
    new_dict->make_ref();
    
    return new_dict;
};

// Dictionary.contains：判断是否包含指定键（key: String），返回Bool
Object* dict_contains(Object* self, const List* args) {
    kiz::Vm::assert_argc(1, args);
    
    auto self_dict = dynamic_cast<Dictionary*>(self);
    assert(self_dict != nullptr);
    
    // 键
    auto key_obj = args->val[0];
    dep::BigInt key_hash_val = hash_object(key_obj);

    auto found_pair_it = self_dict->val.find(
        key_hash_val
    );

    if (found_pair_it) {
        return load_true();
    }
    return load_false();
};

Object* dict_setitem(Object* self, const List* args) {
    kiz::Vm::assert_argc(2, args);
    auto self_dict = dynamic_cast<Dictionary*>(self);
    auto key_obj = args->val[0];
    auto value_obj = args->val[1];
    dep::BigInt key_hash_val = hash_object(key_obj);

    self_dict->val.insert(
        key_hash_val,
        std::pair{key_obj, value_obj}
    );
    return load_nil();
}

Object* dict_getitem(Object* self, const List* args) {
    auto self_dict = dynamic_cast<Dictionary*>(self);
    auto key_obj = builtin::get_one_arg(args);

    dep::BigInt key_hash_val = hash_object(key_obj);

    auto found_pair_it = self_dict->val.find(key_hash_val);
    if (found_pair_it) {
        return found_pair_it->value.second;
    }

    throw NativeFuncError("KeyError",
            "Undefined key " + key_obj->debug_string() + " in Dictionary object " + self->debug_string()
    );
}


Object* dict_str(Object* self, const List* args) {
    auto self_dict = dynamic_cast<Dictionary*>(self);
    std::string result = "{";
    auto kv_list = self_dict->val.to_vector();
    size_t i = 0;
    for (auto& kv_pair : kv_list | std::views::values) {
        result += kiz::Vm::obj_to_str(kv_pair.first) + ": " + kiz::Vm::obj_to_str(kv_pair.second);
        if (i != kv_list.size() - 1) {
            result += ", ";
        }
        ++i;
    }
    result += "}";
    return new String(result);
}

Object* dict_dstr(Object* self, const List* args) {
    auto self_dict = dynamic_cast<Dictionary*>(self);
    std::string result = "{";
    auto kv_list = self_dict->val.to_vector();
    size_t i = 0;
    for (auto& kv_pair : kv_list | std::views::values) {
        result += kiz::Vm::obj_to_debug_str(kv_pair.first) + ": " + kiz::Vm::obj_to_debug_str(kv_pair.second);
        if (i != kv_list.size() - 1) {
            result += ", ";
        }
        ++i;
    }
    result += "}";
    return new String(result);
}

}  // namespace model