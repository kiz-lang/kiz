#include "models.hpp"

namespace model {

//  List.__add__：拼接另一个List（self + 传入List，返回新List）
model::Object* list_add(model::Object* self, const model::List* args) {
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
model::Object* list_mul(model::Object* self, const model::List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (list_mul)");
    assert(args->val.size() == 1 && "function List.mul need 1 arg");
    
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr && "list_mul must be called by List object");
    
    auto times_int = dynamic_cast<Int*>(args->val[0]);
    assert(times_int != nullptr && "List.mul only supports Int type argument");
    assert(times_int->val >= deps::BigInt(0) && "List.mul requires non-negative integer argument");
    
    std::vector<Object*> new_vals;
    deps::BigInt times = times_int->val;
    for (deps::BigInt i = deps::BigInt(0); i < times; i+=deps::BigInt(1)) {
        new_vals.insert(new_vals.end(), self_list->val.begin(), self_list->val.end());
    }
    
    return new List(std::move(new_vals));
};

// List.__eq__：判断两个List是否相等
model::Object* list_eq(model::Object* self, const model::List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (list_eq)");
    assert(args->val.size() == 1 && "function List.eq need 1 arg");
    
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr && "list_eq must be called by List object");
    
    auto another_list = dynamic_cast<List*>(args->val[0]);
    assert(another_list != nullptr && "List.eq only supports List type argument");
    
    // 比较元素个数，不同直接返回false
    if (self_list->val.size() != another_list->val.size()) {
        return new Bool(false);
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
        const auto eq_result = kiz::Vm::get_return_val();

        // 解析比较结果
        const auto eq_bool = dynamic_cast<Bool*>(eq_result);
        assert(eq_bool != nullptr && "__eq__ method must return Bool type");
        
        // 任意元素不相等，返回 false
        if (!eq_bool->val) {
            return new Bool(false);
        }
    }
    
    // 所有元素均相等，返回 true
    return new Bool(true);
};

// List.__contains__：判断列表是否包含目标元素
model::Object* list_contains(model::Object* self, const model::List* args) {
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
        const auto result = kiz::Vm::get_return_val();
    
        const auto result_val = dynamic_cast<Bool*>(result);
        assert(result_val!=nullptr);

        // 找到匹配元素，立即返回true
        if (result_val->val == true) {
            result_val->make_ref();
            return result_val;
        }
    }
    
    // 遍历完未找到匹配元素，返回false
    return new Bool(false);
};

// List.append：向列表尾部添加一个元素
model::Object* list_append(model::Object* self, const model::List* args) {
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

}  // namespace model