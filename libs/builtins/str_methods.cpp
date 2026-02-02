#include "../deps/u8str.hpp"
#include "../../src/models/models.hpp"
#include "include/builtin_functions.hpp"

namespace model {

// String.__call__
Object* str_call(Object* self, const List* args) {
    std::string val;
    if (args->val.empty()) {
        val = "";
    } else {
        val = kiz::Vm::obj_to_str(args->val[0]);
    }
    return create_str(val);
}

// String.__bool__
Object* str_bool(Object* self, const List* args) {
    const auto self_int = dynamic_cast<String*>(self);
    if (self_int->val.empty()) return new Bool(false);
    return load_true();
}

// String.__add__：字符串拼接（self + 传入String，返回新String，不修改原对象）
Object* str_add(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (str_add)");
    assert(args->val.size() == 1 && "function String.add need 1 arg");
    
    auto self_str = dynamic_cast<String*>(self);
    assert(self_str != nullptr && "str_add must be called by String object");
    
    auto another_str = dynamic_cast<String*>(args->val[0]);
    assert(another_str != nullptr && "String.add only supports String type argument");
    
    // 拼接并返回新String
    return create_str(self_str->val + another_str->val);
};

// String.__mul__：字符串重复n次（self * n，返回新String，n为非负整数）
Object* str_mul(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (str_mul)");
    assert(args->val.size() == 1 && "function String.mul need 1 arg");
    
    auto self_str = dynamic_cast<String*>(self);
    assert(self_str != nullptr && "str_mul must be called by String object");
    
    auto times_int = dynamic_cast<Int*>(args->val[0]);
    assert(times_int != nullptr && "String.mul only supports Int type argument");
    assert(times_int->val >= dep::BigInt(0) && "String.mul requires non-negative integer argument");
    
    std::string result;
    dep::BigInt times = times_int->val;
    for (dep::BigInt i = dep::BigInt(0); i < times; i+=dep::BigInt(1)) {
        result += self_str->val;
    }
    
    return new String(std::move(result));
};

// String.__eq__：判断两个字符串是否相等 self == x
Object* str_eq(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (str_eq)");
    assert(args->val.size() == 1 && "function String.eq need 1 arg");
    
    auto self_str = dynamic_cast<String*>(self);
    assert(self_str != nullptr && "str_eq must be called by String object");
    
    auto another_str = dynamic_cast<String*>(args->val[0]);
    assert(another_str != nullptr && "String.eq only supports String type argument");
    
    return new Bool(self_str->val == another_str->val);
};

// String.__contains__：判断是否包含子字符串 x in self
Object* str_contains(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (str_contains)");
    assert(args->val.size() == 1 && "function String.contains need 1 arg");
    
    auto self_str = dynamic_cast<String*>(self);
    assert(self_str != nullptr && "str_contains must be called by String object");
    
    auto sub_str = dynamic_cast<String*>(args->val[0]);
    assert(sub_str != nullptr && "String.contains only supports String type argument");
    
    bool exists = self_str->val.find(sub_str->val) != std::string::npos;
    return new Bool(exists);
};

// String.__hash__
Object* str_hash(Object* self, const List* args) {
    auto self_str = dynamic_cast<String*>(self);
    assert(self_str != nullptr && "str_hash must be called by String object");
    auto hashed_str = dep::hash_string(self_str->val);
    return new Int(dep::BigInt(hashed_str));
}

Object* str_next(Object* self, const List* args) {
    auto curr_idx_it = self->attrs.find("__current_index__");
    assert(curr_idx_it != nullptr);

    auto curr_idx = curr_idx_it->value;

    auto index = cast_to_int(curr_idx) ->val.to_unsigned_long_long();

    auto self_str = dynamic_cast<String*>(self);
    if (index < self_str->val.size()) {
        auto res = dep::UTF8String(self_str->val)[index];
        self->attrs.insert("__current_index__", new Int(index+1));
        return create_str(res.to_string());
    }
    self->attrs.insert("__current_index__", new Int(0));
    return new Bool(false);
}

Object* str_str(Object* self, const List* args) {
    auto self_str = dynamic_cast<String*>(self);
    return create_str(self_str->val);
}

Object* str_dstr(Object* self, const List* args) {
    auto self_str = dynamic_cast<String*>(self);
    return create_str("\"" + self_str->val + "\"");
}

Object* str_getitem(Object* self, const List* args) {
    auto self_str = dynamic_cast<String*>(self);
    auto idx_obj = cast_to_int(builtin::get_one_arg(args));
    auto index = idx_obj->val.to_unsigned_long_long();

    return create_str( dep::UTF8String(self_str->val)[index] .to_string() );
}

Object* str_foreach(Object* self, const List* args) {
    const auto func_obj = builtin::get_one_arg(args);

    auto self_str = cast_to_str(self);

    dep::BigInt idx = 0;
    for (const auto& e : dep::UTF8String(self_str->val)) {
        kiz::Vm::call_function(func_obj, new List({
            create_str(e.to_string())
        }), nullptr);
        idx += 1;
    }
    return new Nil();
}

Object* str_count(Object* self, const List* args) {
    const auto obj = builtin::get_one_arg(args);
    size_t count = 0;
    auto self_str = cast_to_str(self);

    for (const auto& c : dep::UTF8String(self_str->val)) {
        kiz::Vm::call_method(obj, "__eq__", new List({
            create_str(c.to_string())
        }));
        auto res = kiz::Vm::fetch_one_from_stack_top();
        if (res) {
            ++ count;
        }
    }
    return create_int(count);
}


Object* str_startswith(Object* self, const List* args) {
    return new Nil();

}

Object* str_endswith(Object* self, const List* args) {
    return new Nil();

}

Object* str_len(Object* self, const List* args) {
    auto self_str = cast_to_str(self);

    return create_int(dep::UTF8String(self_str->val).size());
}

Object* str_is_alaph(Object* self, const List* args) {
    auto self_str = cast_to_str(self);
    auto str = dep::UTF8String(self_str->val);
    bool is_alaph = true;
    for (const auto& c : str) {
        if (!c.is_alpha()) is_alaph = false;
    }
    return load_bool(is_alaph);
}

Object* str_is_digit(Object* self, const List* args) {
    auto self_str = cast_to_str(self);
    auto str = dep::UTF8String(self_str->val);
    bool is_digit = true;
    for (const auto& c : str) {
        if (!c.is_digit()) is_digit = false;
    }
    return load_bool(is_digit);

}

Object* str_substr(Object* self, const List* args) {
    auto self_str = cast_to_str(self);
    auto args_vec = args->val;
    assert(!args_vec.empty());

    size_t pos = cast_to_int(args_vec[0])->val.to_unsigned_long_long();
    size_t len = 1;
    if (args_vec.size() > 1) {
        len = cast_to_int(args_vec[1])->val.to_unsigned_long_long();
    }

    return create_str(dep::UTF8String(self_str->val).substr(
        pos, len
    ).to_string());
}

Object* str_to_lower(Object* self, const List* args) {
    auto self_str = cast_to_str(self);

    return create_str(dep::UTF8String(self_str->val).to_lower().to_string());
}

Object* str_to_upper(Object* self, const List* args) {
    auto self_str = cast_to_str(self);

    return create_str(dep::UTF8String(self_str->val).to_upper().to_string());

}


}  // namespace model