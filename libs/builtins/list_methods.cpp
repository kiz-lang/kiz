#include "../../src/models/models.hpp"
#include "include/builtin_functions.hpp"

namespace model {

// List.__call__
Object* list_call(Object* self, const List* args) {
    auto obj = create_list({});
    std::vector<Object*> list = {};
    if (args->val.empty()) {
        return obj;
    }

    auto for_cast = builtin::get_one_arg(args);
    while (true) {
        kiz::Vm::call_method(for_cast, "__next__", new List({}));
        auto res = kiz::Vm::fetch_one_from_stack_top();
        if (res == stop_iter_signal) {
            break;
        }
        list.push_back(res);
    }
    obj->val = list;
    return obj;
}

// List.__bool__
Object* list_bool(Object* self, const List* args) {
    const auto self_int = dynamic_cast<List*>(self);
    if (self_int->val.empty()) return load_false();
    return load_true();
}

//  List.__add__：拼接另一个List（self + 传入List，返回新List）
Object* list_add(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (list_add)");
    assert(args->val.size() == 1 && "function List.add need 1 arg");
    
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr && "list_add must be called by List object");
    
    auto another_list = dynamic_cast<List*>(args->val[0]);
    assert(another_list != nullptr && "List.add only supports List type argument");
    
    // 浅拷贝
    std::vector<Object*> new_vals = self_list->val;
    new_vals.insert(new_vals.end(), another_list->val.begin(), another_list->val.end());
    
    return new List(std::move(new_vals));
};

// List.__mul__：重复自身n次 self * n
Object* list_mul(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (list_mul)");
    assert(args->val.size() == 1 && "function List.mul need 1 arg");
    
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr && "list_mul must be called by List object");
    
    auto times_int = dynamic_cast<Int*>(args->val[0]);
    assert(times_int != nullptr && "List.mul only supports Int type argument");
    assert(times_int->val >= dep::BigInt(0) && "List.mul requires non-negative integer argument");
    
    std::vector<Object*> new_vals;
    dep::BigInt times = times_int->val;
    for (dep::BigInt i = dep::BigInt(0); i < times; i+=dep::BigInt(1)) {
        new_vals.insert(new_vals.end(), self_list->val.begin(), self_list->val.end());
    }
    
    return new List(std::move(new_vals));
};

// List.__eq__：判断两个List是否相等
Object* list_eq(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (list_eq)");
    assert(args->val.size() == 1 && "function List.eq need 1 arg");
    
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr && "list_eq must be called by List object");
    
    auto another_list = dynamic_cast<List*>(args->val[0]);
    assert(another_list != nullptr && "List.eq only supports List type argument");
    
    // 比较元素个数，不同直接返回false
    if (self_list->val.size() != another_list->val.size()) {
        return load_false();
    }
    
    // 逐个比较元素
    for (size_t i = 0; i < self_list->val.size(); ++i) {
        Object* self_elem = self_list->val[i];
        Object* another_elem = another_list->val[i];

        // 获取当前元素的 __eq__ 方法
        const auto elem_eq_method = kiz::Vm::get_attr(self_elem, "__eq__");
        assert(elem_eq_method != nullptr && "Element must implement __eq__ method");
        
        // 调用 __eq__
        kiz::Vm::call_function(
            elem_eq_method, new List({another_elem}), self_elem
        );
        const auto eq_result = kiz::Vm::fetch_one_from_stack_top();

        // 解析比较结果
        const auto eq_bool = dynamic_cast<Bool*>(eq_result);
        assert(eq_bool != nullptr && "__eq__ method must return Bool type");
        
        // 任意元素不相等，返回 false
        if (!eq_bool->val) {
            return load_false();
        }
    }
    
    // 所有元素均相等，返回 true
    return load_true();
};

Object* list_str(Object* self, const List* args) {
    auto self_list = dynamic_cast<List*>(self);
    std::string result = "[";
    for (size_t i = 0; i < self_list->val.size(); ++i) {
        if (self_list->val[i] != nullptr) {
            result += kiz::Vm::obj_to_str(self_list->val[i]);
        } else {
            result += "Nil";
        }
        if (i != self_list->val.size() - 1) {
            result += ", ";
        }
    }
    result += "]";
    return create_str(result);
}

Object* list_dstr(Object* self, const List* args) {
    auto self_list = dynamic_cast<List*>(self);
    std::string result = "[";
    for (size_t i = 0; i < self_list->val.size(); ++i) {
        if (self_list->val[i] != nullptr) {
            result += kiz::Vm::obj_to_debug_str(self_list->val[i]);
        } else {
            result += "Nil";
        }
        if (i != self_list->val.size() - 1) {
            result += ", ";
        }
    }
    result += "]";
    return create_str(result);
}

// List.contains：判断列表是否包含目标元素
Object* list_contains(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (list_contains)");
    assert(args->val.size() == 1 && "function List.contains need 1 arg");
    
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr && "list_contains must be called by List object");
    
    Object* target_elem = args->val[0];
    assert(target_elem != nullptr && "List.contains target argument cannot be nullptr");
    
    // 遍历列表元素，逐个判断是否与目标元素相等
    for (Object* elem : self_list->val) {
        const auto elem_eq_method = kiz::Vm::get_attr(elem, "__eq__");

        kiz::Vm::call_function(
            elem_eq_method, new List({target_elem}), elem
        );
        const auto result = kiz::Vm::fetch_one_from_stack_top();

        // 找到匹配元素，立即返回true
        if (kiz::Vm::is_true(result)) return load_true();
    }
    
    // 遍历完未找到匹配元素，返回false
    return load_false();
};

// List.append：向列表尾部添加一个元素
Object* list_append(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (list_append)");
    assert(args->val.size() == 1 && "function List.append need 1 arg");
    
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr && "list_append must be called by List object");
    
    Object* elem_to_add = args->val[0];
    assert(elem_to_add != nullptr && "List.append argument cannot be nullptr");
    
    // 添加元素到列表尾部
    self_list->val.push_back(elem_to_add);
    elem_to_add->make_ref();
    
    // 返回列表自身，支持链式调用
    self->make_ref();
    return self;
};

Object* list_next(Object* self, const List* args) {
    auto curr_idx_it = self->attrs.find("__current_index__");
    assert(curr_idx_it != nullptr);

    auto curr_idx = curr_idx_it->value;

    auto index =  cast_to_int(curr_idx) ->val.to_unsigned_long_long();

    auto self_list = dynamic_cast<List*>(self);
    if (index < self_list->val.size()) {
        auto res = self_list->val[index];
        self->attrs.insert("__current_index__", create_int(index+1));
        return res;
    }
    self->attrs.insert("__current_index__", create_int(0));
    return stop_iter_signal;
}

Object* list_foreach(Object* self, const List* args) {
    auto func_obj = builtin::get_one_arg(args);

    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr);

    dep::BigInt idx = 0;
    for (auto e : self_list->val) {
        kiz::Vm::call_function(func_obj, new List({e}), nullptr);
        idx += 1;
    }
    return load_nil();
}

Object* list_reverse(Object* self, const List* args) {
    const auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr);
    std::ranges::reverse(self_list->val);
    return load_nil();
}

Object* list_extend(Object* self, const List* args) {
    auto other_list_obj = builtin::get_one_arg(args);
    auto other_list = dynamic_cast<List*>(other_list_obj);
    assert(other_list != nullptr);

    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr);
    for (auto e: other_list->val) {
        self_list->val.push_back(e);
    }
    return load_nil();
}

Object* list_pop(Object* self, const List* args) {
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr);
    self_list->val.pop_back();
    return load_nil();
}

Object* list_insert(Object* self, const List* args) {
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr);
    if (args->val.size() == 2) {
        auto value_obj = args->val[0];
        auto idx_obj = args->val[1];
        auto idx_int = dynamic_cast<Int*>(idx_obj);
        assert(idx_int != nullptr);
        auto idx = idx_int->val.to_unsigned_long_long();
        if (idx < self_list->val.size()) {
            self_list->val[idx] = value_obj;
        }
    }
    return load_nil();
}

Object* list_setitem(Object* self, const List* args) {
    assert(args->val.size() == 2);
    auto self_list = dynamic_cast<List*>(self);
    auto idx_obj = dynamic_cast<Int*>(args->val[0]);
    assert(idx_obj != nullptr);
    auto index = idx_obj->val.to_unsigned_long_long();

    auto value_obj = args->val[1];

    if (index < self_list->val.size()) {
        self_list->val[index] = value_obj;;
    }
    assert(false);
}

Object* list_getitem(Object* self, const List* args) {
    auto self_list = dynamic_cast<List*>(self);
    auto idx_obj = dynamic_cast<Int*>(builtin::get_one_arg(args));
    assert(idx_obj != nullptr);

    auto index = idx_obj->val.to_unsigned_long_long();
    if (index < self_list->val.size()) {
        return self_list->val[index];
    }
    assert(false);
}

Object* list_count(Object* self, const List* args) {
    const auto obj = builtin::get_one_arg(args);
    size_t count = 0;
    auto self_list = cast_to_list(self);

    for (const auto& item : self_list->val) {
        kiz::Vm::call_method(obj, "__eq__", new List({item}));
        auto res = kiz::Vm::fetch_one_from_stack_top();
        if (res) {
            ++ count;
        }
    }
    return create_int(count);
}

Object* list_find(Object* self, const List* args) {
    auto func_obj = builtin::get_one_arg(args);

    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr);

    for (auto e : self_list->val) {
        kiz::Vm::call_function(func_obj, new List({e}), nullptr);
        auto res = kiz::Vm::fetch_one_from_stack_top();
        if (kiz::Vm::is_true(res)) {
            res->make_ref();
            return res;
        }
    }
    return load_nil();
}

Object* list_map(Object* self, const List* args) {
    auto func_obj = builtin::get_one_arg(args);

    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr);

    std::vector<Object*> new_vec;

    for (auto e : self_list->val) {
        kiz::Vm::call_function(func_obj, new List({e}), nullptr);
        auto res = kiz::Vm::fetch_one_from_stack_top();
        
        new_vec.push_back(res);
    }
    return create_list(new_vec);
}

Object* list_filter(Object* self, const List* args) {
    auto func_obj = builtin::get_one_arg(args);

    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr);

    std::vector<Object*> new_vec;

    for (auto e : self_list->val) {
        kiz::Vm::call_function(func_obj, new List({e}), nullptr);
        auto res = kiz::Vm::fetch_one_from_stack_top();
        if (kiz::Vm::is_true(res)) {
            new_vec.push_back(res);
        }
    }
    return create_list(new_vec);
}

Object* list_len(Object* self, const List* args) {
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr);
    return create_int(dep::BigInt(self_list->val.size()));
}

}  // namespace model