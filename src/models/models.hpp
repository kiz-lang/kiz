/**
 * @file models.hpp
 * @brief 虚拟机对象模型（VM Models）核心定义
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once

#include <atomic>
#include <functional>
#include <iomanip>
#include <ranges>
#include <utility>

#include "../kiz.hpp"
#include "../vm/vm.hpp"
#include "../../deps/hashmap.hpp"
#include "../../deps/bigint.hpp"
#include "../../deps/decimal.hpp"
#include "../../deps/dict.hpp"

namespace model {

namespace magic_name {
    constexpr auto add = "__add__";
    constexpr auto sub = "__sub__";
    constexpr auto mul = "__mul__";
    constexpr auto div = "__div__";
    constexpr auto pow = "__pow__";
    constexpr auto mod = "__mod__";
    constexpr auto eq = "__eq__";
    constexpr auto lt = "__lt__";
    constexpr auto gt = "__gt__";

    constexpr auto parent = "__parent__";
    constexpr auto call = "__call__";
    constexpr auto bool_of = "__bool__";
    constexpr auto str = "__str__";
    constexpr auto debug_str = "__dstr__";
    constexpr auto getitem = "__getitem__";
    constexpr auto setitem = "__setitem__";
    constexpr auto contains = "contains";
    constexpr auto next_item = "__next__";
    constexpr auto hash = "__hash__";
    constexpr auto owner_module = "__owner_module__";
}

// 工具函数ptr转为地址的字符串
template <typename T>
std::string ptr_to_string(T* m) {
    // 将指针转为 uintptr_t
    auto ptr_val = reinterpret_cast<uintptr_t>(m);

    // 格式化字符串
    std::stringstream ss;
    ss << "0x" << std::hex << std::setfill('0')
        << std::setw(sizeof(uintptr_t) * 2)
        << ptr_val;

    return ss.str();
}

class Object {
    std::atomic<size_t> refc_ = 0;
public:
    dep::HashMap<Object*> attrs;

    // 对象类型枚举
    enum class ObjectType {
        OT_Object, OT_Nil, OT_Bool, OT_Int, OT_Rational, OT_String,
        OT_List, OT_Dictionary, OT_CodeObject, OT_Function,
        OT_CppFunction, OT_Module, OT_Error, OT_Decimal
    };

    // 获取实际类型的虚函数
    [[nodiscard]] virtual ObjectType get_type() const {
        return ObjectType::OT_Object;
    }

    [[nodiscard]] size_t get_refc_() const {
        return refc_;
    }

    void make_ref() {
        std::cout << "adding refc: " << this->debug_string() << " " << refc_  << " -> " << refc_+1 << std::endl;
        refc_.fetch_add(1, std::memory_order_relaxed);
    }
    void del_ref() {
        std::cout << "deling refc: " << this->debug_string() << " " << refc_  << " -> " << refc_-1 << std::endl;
        const size_t old_ref = refc_.fetch_sub(1, std::memory_order_acq_rel);

        if (old_ref == 1) {
            std::cout << "deling object " << this->debug_string() << std::endl;
            delete this;
        }
    }

    [[nodiscard]] virtual std::string debug_string() const {
        return "<Object at " + ptr_to_string(this) + ">";
    }

    Object () {}

    virtual ~Object() {
        // auto kv_list = attrs.to_vector();
        // for (auto& obj : kv_list | std::views::values) {
        //     if (obj) obj->del_ref();
        // }
    }
};

inline auto based_obj = new Object();
inline auto based_list = new Object();
inline auto based_function = new Object();
inline auto based_dict = new Object();
inline auto based_int = new Object();
inline auto based_bool = new Object();
inline auto based_nil = new Object();
inline auto based_str = new Object();
inline auto based_native_function = new Object();
inline auto based_error = new Object();
inline auto based_decimal = new Object();
inline auto based_module = new Object();
inline auto stop_iter_signal = new Object();

class List;

class CodeObject : public Object {
public:
    std::vector<kiz::Instruction> code;
    std::vector<std::string> names;

    static constexpr ObjectType TYPE = ObjectType::OT_CodeObject;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit CodeObject(const std::vector<kiz::Instruction>& code,
        const std::vector<std::string>& names
    ) : code(code), names(names) {}

    [[nodiscard]] std::string debug_string() const override {
        return "<CodeObject at " + ptr_to_string(this) + ">";
    }
};

class Module : public Object {
public:
    std::string path;
    CodeObject* code = nullptr;

    static constexpr ObjectType TYPE = ObjectType::OT_Module;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Module(std::string name, CodeObject *code) : path(std::move(name)), code(code) {
        attrs.insert("__parent__", based_module);
        code->make_ref();
    }

    explicit Module(std::string name) : path(std::move(name)) {
        attrs.insert("__parent__", based_module);
    }

    [[nodiscard]] std::string debug_string() const override {
        return "<Module: path='" + path + "', attr=" + attrs.to_string() + ", at " + ptr_to_string(this) + ">";
    }

    ~Module() override {
        code->del_ref();
    }
};

class Function : public Object {
public:
    std::string name;
    CodeObject* code = nullptr;
    size_t argc = 0;
    bool has_rest_params = false;

    static constexpr ObjectType TYPE = ObjectType::OT_Function;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Function(std::string name, CodeObject *code, const size_t argc
    ) : name(std::move(name)), code(code), argc(argc) {
        code->make_ref();
        attrs.insert("__parent__", based_function);
    }

    [[nodiscard]] std::string debug_string() const override {
        return "<Function: path='" + name + "', argc=" + std::to_string(argc) + " at " + ptr_to_string(this) + ">";
    }

    ~Function() override {
        code->del_ref();
    }
};

class NativeFunction : public Object {
public:
    std::string name;
    std::function<Object*(Object*, List*)> func;

    static constexpr ObjectType TYPE = ObjectType::OT_CppFunction;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit NativeFunction(std::function<Object*(Object*, List*)> func) : func(std::move(func)) {
        attrs.insert("__parent__", based_native_function);
    }
    [[nodiscard]] std::string debug_string() const override {
    return "<NativeFunction" +
           (name.empty() 
            ? "" 
            : ": path='" + name + "'"
            ) 
            + " at " + ptr_to_string(this) + ">";
}
};

class Int : public Object {
public:
    dep::BigInt val;

    static constexpr ObjectType TYPE = ObjectType::OT_Int;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Int(dep::BigInt val) : val(std::move(val)) {
        attrs.insert("__parent__", based_int);
    }
    explicit Int() : val(dep::BigInt(0)) {
        attrs.insert("__parent__", based_int);
    }
    [[nodiscard]] std::string debug_string() const override {
        return val.to_string();
    }
};

class List : public Object {
public:
    std::vector<Object*> val;

    static constexpr ObjectType TYPE = ObjectType::OT_List;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit List(std::vector<Object*> val) : val(std::move(val)) {
        attrs.insert("__parent__", based_list);
        auto zero = new Int(0);
        zero->make_ref();
        attrs.insert("__current_index__", zero);
    }
    [[nodiscard]] std::string debug_string() const override {
        std::string result = "[";
        for (size_t i = 0; i < val.size(); ++i) {
            if (val[i] != nullptr) {
                result += val[i]->debug_string();  // 递归调用元素的 to_string
            } else {
                result += "Nil";
            }
            if (i != val.size() - 1) {
                result += ", ";
            }
        }
        result += "]";
        return result;
    }

    ~List() override {
        for (auto elem : val) {
            if (elem) elem->del_ref();
        }
    }
};

class Decimal : public Object {
public:
    dep::Decimal val;
    static constexpr ObjectType TYPE = ObjectType::OT_Decimal;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }
    explicit Decimal(dep::Decimal val) : val(std::move(val)) {
        attrs.insert("__parent__", based_decimal);
    }
    [[nodiscard]] std::string debug_string() const override {
        return val.to_string();
    }
};

class String : public Object {
public:
    std::string val;

    static constexpr ObjectType TYPE = ObjectType::OT_String;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit String(std::string val) : val(std::move(val)) {
        attrs.insert("__parent__", based_str);
        auto zero = new Int(0);
        zero->make_ref();
        attrs.insert("__current_index__", zero);
    }
    [[nodiscard]] std::string debug_string() const override {
        return '"'+val+'"';
    }
};

class Dictionary : public Object {
public:
    dep::Dict<std::pair<Object*, Object*>> val;
    static constexpr ObjectType TYPE = ObjectType::OT_Dictionary;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Dictionary(dep::Dict<std::pair<Object*, Object*>> val_) : val(std::move(val_)) {
        attrs.insert("__parent__", based_dict);
    }
    explicit Dictionary() {
        attrs.insert("__parent__", based_dict);
    }

    [[nodiscard]] std::string debug_string() const override {
        std::string result = "{";
        auto kv_list = val.to_vector();
        size_t i = 0;
        for (auto& [_, kv_pair] : kv_list) {
            result += kv_pair.first->debug_string() + ": " + kv_pair.second->debug_string();
            if (i != kv_list.size() - 1) {
                result += ", ";
            }
            ++i;
        }
        result += "}";
        return result;
    }

    ~Dictionary() override {
        auto kv_list = val.to_vector();
        for (auto& [_, kv_pair] : kv_list) {
            if (kv_pair.first) kv_pair.first->del_ref();
            if (kv_pair.second) kv_pair.second->del_ref();
        }
    }
};

class Bool : public Object {
public:
    bool val;

    static constexpr ObjectType TYPE = ObjectType::OT_Bool;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Bool(const bool val) : val(val) {
        attrs.insert("__parent__", based_bool);
    }
    [[nodiscard]] std::string debug_string() const override {
        return val ? "True" : "False";
    }
};

class Nil : public Object {
public:

    static constexpr ObjectType TYPE = ObjectType::OT_Nil;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Nil() : Object() {
        attrs.insert("__parent__", based_nil);
    }
    [[nodiscard]] std::string debug_string() const override {
        return "Nil";
    }
};

class Error : public Object {
public:
    std::vector<std::pair<std::string, err::PositionInfo>> positions;
    static constexpr ObjectType TYPE = ObjectType::OT_Error;
    [[nodiscard]] ObjectType get_type() const override { return TYPE; }

    explicit Error(std::vector<std::pair<std::string, err::PositionInfo>> p) {
        positions = std::move(p);
        attrs.insert("__parent__", based_error);
    }

    explicit Error() {
        attrs.insert("__parent__", based_error);
    }

    [[nodiscard]] std::string debug_string() const override {
        return "Error";
    }
};

inline auto unique_nil = new Nil();
inline auto unique_false = new Bool(false);
inline auto unique_true = new Bool(true);

inline auto load_nil() {
    unique_nil->make_ref();
    return unique_nil;
}
inline auto load_false() {
    unique_false->make_ref();
    return unique_false;
}
inline auto load_true() {
    unique_true->make_ref();
    return unique_true;
}

inline auto load_bool(bool b) {
    if (b) { return load_true(); }
    return load_false();
}

inline auto create_int(dep::BigInt n) {
    auto o = new Int(std::move(n));
    o->make_ref();
    return o;
}

inline auto create_str(std::string n) {
    auto o = new String(std::move(n));
    o->make_ref();
    return o;
}

inline auto create_decimal(dep::Decimal n) {
    auto o = new Decimal(std::move(n));
    o->make_ref();
    return o;
}

inline auto create_list(std::vector<Object*> n) {
    auto o = new List(std::move(n));
    o->make_ref();
    return o;
}

inline auto create_nfunc(const std::function<Object*(Object*, List*)>& func, const std::string& name="<unnamed>") {
    auto o = new NativeFunction(func);
    o->make_ref();
    o->name = name;
    return o;
}

inline auto cast_to_int(Object* o) {
    auto obj = dynamic_cast<Int*>(o);
    assert(obj != nullptr);
    return obj;
}

inline auto cast_to_str(Object* o) {
    auto obj = dynamic_cast<String*>(o);
    assert(obj != nullptr);
    return obj;
}

inline auto cast_to_bool(Object* o) {
    auto obj = dynamic_cast<Bool*>(o);
    assert(obj != nullptr);
    return obj;
}

inline auto cast_to_list(Object* o) {
    auto obj = dynamic_cast<List*>(o);
    assert(obj != nullptr);
    return obj;
}

inline auto copy_or_ref(Object* obj) -> Object* {
    switch (obj->get_type()) {

    case Object::ObjectType::OT_List: {
        std::vector<Object*> new_val;
        for (auto val : cast_to_list(obj)->val) {
            new_val.push_back(copy_or_ref(val));
        }
        return create_list(std::move(new_val));
    }

    case Object::ObjectType::OT_Dictionary: {
        std::vector<std::pair<
            dep::BigInt, std::pair< Object*, Object* >
        >> elem_list;
        auto dict_obj = dynamic_cast<Dictionary*>(obj);
        assert(dict_obj != nullptr);
        for (auto& [_, kv_pair] : dict_obj->val.to_vector()) {
            // key是hashable value, 也就是不可变对象, 可以引用传递, 应该没有神人为可变对象重载__hash__方法的
            elem_list.emplace_back(_, std::pair{
                kv_pair.first, copy_or_ref(kv_pair.second)
            });
        }
        auto new_dict_obj = new Dictionary(dep::Dict(elem_list));
        new_dict_obj->make_ref();
        return new_dict_obj;
    }

    default: {
        obj->make_ref();
        return obj;
    }

    }
}

};