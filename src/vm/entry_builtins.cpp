#include <format>

#include "vm.hpp"
#include "../models/models.hpp"
#include "builtins/include/builtin_functions.hpp"
#include "builtins/include/builtin_methods.hpp"

namespace kiz {

void Vm::entry_builtins() {
    builtins.insert("print", model::create_nfunc(builtin::print, "print"));
    builtins.insert("input", model::create_nfunc(builtin::input, "input"));
    builtins.insert("ischild", model::create_nfunc(builtin::ischild, "ischild"));
    builtins.insert("create", model::create_nfunc(builtin::create, "create"));
    builtins.insert("now", model::create_nfunc(builtin::now, "now"));
    builtins.insert("get_refc", model::create_nfunc(builtin::get_refc, "get_refc"));
    builtins.insert("breakpoint", model::create_nfunc(builtin::breakpoint, "breakpoint"));
    builtins.insert("cmd", model::create_nfunc(builtin::cmd, "cmd"));
    builtins.insert("help", model::create_nfunc(builtin::help, "help"));
    builtins.insert("delattr", model::create_nfunc(builtin::delattr, "delattr"));
    builtins.insert("setattr", model::create_nfunc(builtin::setattr, "setattr"));
    builtins.insert("getattr", model::create_nfunc(builtin::getattr, "getattr"));
    builtins.insert("hasattr", model::create_nfunc(builtin::hasattr, "hasattr"));
    builtins.insert("range", model::create_nfunc(builtin::range, "range"));
    builtins.insert("type_of", model::create_nfunc(builtin::type_of_obj, "type_of"));


    DEBUG_OUTPUT("registering builtin objects...");
    builtins.insert("Object", model::based_obj);

    model::based_bool->attrs.insert("__parent__", model::based_obj);
    model::based_int->attrs.insert("__parent__", model::based_obj);
    model::based_nil->attrs.insert("__parent__", model::based_obj);
    model::based_function->attrs.insert("__parent__", model::based_obj);
    model::based_decimal->attrs.insert("__parent__", model::based_obj);
    model::based_module->attrs.insert("__parent__", model::based_obj);
    model::based_dict->attrs.insert("__parent__", model::based_obj);
    model::based_list->attrs.insert("__parent__", model::based_obj);
    model::based_native_function->attrs.insert("__parent__", model::based_obj);
    model::based_error->attrs.insert("__parent__", model::based_obj);
    model::based_str->attrs.insert("__parent__", model::based_obj);
    model::stop_iter_signal->attrs.insert("__parent__", model::based_obj);

    DEBUG_OUTPUT("registering magic methods...");

    // Object 基类 方法
    model::based_obj->attrs.insert("__eq__", model::create_nfunc([](const model::Object* self, const model::List* args) -> model::Object* {
        const auto other_obj = builtin::get_one_arg(args);
        return model::load_bool(self == other_obj);
    }));

    model::based_obj->attrs.insert("__str__", model::create_nfunc([](const model::Object* self, const model::List* args) -> model::Object* {
        return model::create_str("<Object at " + model::ptr_to_string(self) + ">");
    }));

    model::based_obj->attrs.insert("__getitem__", model::create_nfunc([](const model::Object* self, const model::List* args) -> model::Object* {
        auto attr = builtin::get_one_arg(args);
        auto attr_str = dynamic_cast<model::String*>(attr);
        assert(attr_str != nullptr);
        return get_attr(self, attr_str->val);
    }));

    model::based_obj->attrs.insert("__setitem__", model::create_nfunc([](model::Object* self, model::List* args) -> model::Object* {
        assert(args->val.size() == 2);
        auto attr = args->val[0];
        auto attr_str = dynamic_cast<model::String*>(attr);
        assert(attr_str != nullptr);
        self->attrs.insert(attr_str->val, args->val[1]);
        return self;
    }));

    // Bool 类型魔法方法
    model::based_bool->attrs.insert("__eq__", model::create_nfunc(model::bool_eq));
    model::based_bool->attrs.insert("__call__", model::create_nfunc(model::bool_call));
    model::based_bool->attrs.insert("__hash__", model::create_nfunc(model::bool_hash));
    model::based_bool->attrs.insert("__str__", model::create_nfunc(model::bool_str));

    // Nil 类型魔法方法
    model::based_nil->attrs.insert("__eq__", model::create_nfunc(model::nil_eq));
    model::based_nil->attrs.insert("__hash__", model::create_nfunc(model::nil_hash));
    model::based_nil->attrs.insert("__str__", model::create_nfunc(model::nil_str));

    // Int 类型魔法方法
    model::based_int->attrs.insert("__add__", model::create_nfunc(model::int_add));
    model::based_int->attrs.insert("__sub__", model::create_nfunc(model::int_sub));
    model::based_int->attrs.insert("__mul__", model::create_nfunc(model::int_mul));
    model::based_int->attrs.insert("__div__", model::create_nfunc(model::int_div));
    model::based_int->attrs.insert("__mod__", model::create_nfunc(model::int_mod));
    model::based_int->attrs.insert("__pow__", model::create_nfunc(model::int_pow));
    model::based_int->attrs.insert("__neg__", model::create_nfunc(model::int_neg));
    model::based_int->attrs.insert("__gt__", model::create_nfunc(model::int_gt));
    model::based_int->attrs.insert("__lt__", model::create_nfunc(model::int_lt));
    model::based_int->attrs.insert("__eq__", model::create_nfunc(model::int_eq));
    model::based_int->attrs.insert("__call__", model::create_nfunc(model::int_call));
    model::based_int->attrs.insert("__bool__", model::create_nfunc(model::int_bool));
    model::based_int->attrs.insert("__hash__", model::create_nfunc(model::int_hash));
    model::based_int->attrs.insert("__str__", model::create_nfunc(model::int_str));

    // Decimal类型魔术方法
    model::based_decimal->attrs.insert("__add__", model::create_nfunc(model::decimal_add));
    model::based_decimal->attrs.insert("__sub__", model::create_nfunc(model::decimal_sub));
    model::based_decimal->attrs.insert("__mul__", model::create_nfunc(model::decimal_mul));
    model::based_decimal->attrs.insert("__div__", model::create_nfunc(model::decimal_div));
    model::based_decimal->attrs.insert("__pow__", model::create_nfunc(model::decimal_pow));
    model::based_decimal->attrs.insert("__neg__", model::create_nfunc(model::decimal_neg));
    model::based_decimal->attrs.insert("__gt__", model::create_nfunc(model::decimal_gt));
    model::based_decimal->attrs.insert("__lt__", model::create_nfunc(model::decimal_lt));
    model::based_decimal->attrs.insert("__eq__", model::create_nfunc(model::decimal_eq));
    model::based_decimal->attrs.insert("__call__", model::create_nfunc(model::decimal_call));
    model::based_decimal->attrs.insert("__bool__", model::create_nfunc(model::decimal_bool));
    model::based_decimal->attrs.insert("__hash__", model::create_nfunc(model::decimal_hash));
    model::based_decimal->attrs.insert("__str__", model::create_nfunc(model::decimal_str));
    model::based_decimal->attrs.insert("safe_div", model::create_nfunc(model::decimal_safe_div));

    // Dictionary 类型魔法方法
    model::based_dict->attrs.insert("__add__", model::create_nfunc(model::dict_add));
    model::based_dict->attrs.insert("__contains__", model::create_nfunc(model::dict_contains));
    model::based_dict->attrs.insert("__getitem__", model::create_nfunc(model::dict_getitem));
    model::based_dict->attrs.insert("__str__", model::create_nfunc(model::dict_str));
    model::based_dict->attrs.insert("__dstr__", model::create_nfunc(model::dict_dstr));
    model::based_dict->attrs.insert("__setitem__", model::create_nfunc(model::dict_setitem));

    // List 类型魔法方法
    model::based_list->attrs.insert("__add__", model::create_nfunc(model::list_add));
    model::based_list->attrs.insert("__mul__", model::create_nfunc(model::list_mul));
    model::based_list->attrs.insert("__eq__", model::create_nfunc(model::list_eq));
    model::based_list->attrs.insert("__call__", model::create_nfunc(model::list_call));
    model::based_list->attrs.insert("__bool__", model::create_nfunc(model::list_bool));
    model::based_list->attrs.insert("__next__", model::create_nfunc(model::list_next));
    model::based_list->attrs.insert("__getitem__", model::create_nfunc(model::list_getitem));
    model::based_list->attrs.insert("__setitem__", model::create_nfunc(model::list_setitem));
    model::based_list->attrs.insert("__str__", model::create_nfunc(model::list_str));
    model::based_list->attrs.insert("__dstr__", model::create_nfunc(model::list_dstr));

    model::based_list->attrs.insert("append", model::create_nfunc(model::list_append));
    model::based_list->attrs.insert("contains", model::create_nfunc(model::list_contains));
    model::based_list->attrs.insert("foreach", model::create_nfunc(model::list_foreach));
    model::based_list->attrs.insert("reverse", model::create_nfunc(model::list_reverse));
    model::based_list->attrs.insert("extend", model::create_nfunc(model::list_extend));
    model::based_list->attrs.insert("pop", model::create_nfunc(model::list_pop));
    model::based_list->attrs.insert("insert", model::create_nfunc(model::list_insert));
    model::based_list->attrs.insert("find", model::create_nfunc(model::list_find));
    model::based_list->attrs.insert("map", model::create_nfunc(model::list_map));
    model::based_list->attrs.insert("count", model::create_nfunc(model::list_count));
    model::based_list->attrs.insert("filter", model::create_nfunc(model::list_filter));
    model::based_list->attrs.insert("len", model::create_nfunc(model::list_len));

    // String 类型魔法方法
    model::based_str->attrs.insert("__add__", model::create_nfunc(model::str_add));
    model::based_str->attrs.insert("__mul__", model::create_nfunc(model::str_mul));
    model::based_str->attrs.insert("__eq__", model::create_nfunc(model::str_eq));
    model::based_str->attrs.insert("__call__", model::create_nfunc(model::str_call));
    model::based_str->attrs.insert("__bool__", model::create_nfunc(model::str_bool));
    model::based_str->attrs.insert("__hash__", model::create_nfunc(model::str_hash));
    model::based_str->attrs.insert("__getitem__", model::create_nfunc(model::str_getitem));
    model::based_str->attrs.insert("__str__", model::create_nfunc(model::str_str));
    model::based_str->attrs.insert("__dstr__", model::create_nfunc(model::str_dstr));
    model::based_str->attrs.insert("__next__", model::create_nfunc(model::str_next));

    model::based_str->attrs.insert("contains", model::create_nfunc(model::str_contains));
    model::based_str->attrs.insert("count", model::create_nfunc(model::str_count));
    model::based_str->attrs.insert("foreach", model::create_nfunc(model::str_foreach));
    model::based_str->attrs.insert("startswith", model::create_nfunc(model::str_startswith));
    model::based_str->attrs.insert("endswith", model::create_nfunc(model::str_endswith));
    model::based_str->attrs.insert("substr", model::create_nfunc(model::str_substr));
    model::based_str->attrs.insert("len", model::create_nfunc(model::str_len));
    model::based_str->attrs.insert("is_alaph", model::create_nfunc(model::str_is_alaph));
    model::based_str->attrs.insert("is_digit", model::create_nfunc(model::str_is_digit));
    model::based_str->attrs.insert("to_lower", model::create_nfunc(model::str_to_lower));
    model::based_str->attrs.insert("to_upper", model::create_nfunc(model::str_to_upper));
    model::based_str->attrs.insert("format", model::create_nfunc(model::str_format));


    model::based_error->attrs.insert("__call__", model::create_nfunc([](model::Object* self, model::List* args) {
        assert( args->val.size() == 2);
        auto err_name = args->val[0];
        auto err_msg = args->val[1];

        auto err = new model::Error(gen_pos_info());
        err->attrs.insert("__name__", err_name);
        err->attrs.insert("__msg__", err_msg);
        // std::cout << std::format("throw Err pos f{} c{} l{}",
        //             err->positions.back().first, err->positions.back().second.col_start, err->positions.back().second.lno_start) << std::endl;
        return err;
    }));

    model::based_error->attrs.insert("__str__", model::create_nfunc([](model::Object* self, model::List* args) {
        return model::create_str("Error");
    }));

    model::based_module->attrs.insert("__str__", model::create_nfunc([](model::Object* self, model::List* args){
        auto self_mod = dynamic_cast<model::Module*>(self);
        return model::create_str(
            "<Module: path='" + self_mod->path + "', attr=" + self_mod->attrs.to_string() + ", at " + ptr_to_string(self_mod) + ">"
        );
    }));

    model::based_function->attrs.insert("__str__", model::create_nfunc([](model::Object* self, model::List* args){
       auto self_fn = dynamic_cast<model::Function*>(self);
       return model::create_str(
           "<Function: path='" + self_fn->name + "', argc=" + std::to_string(self_fn->argc) + " at " + ptr_to_string(self_fn) + ">"
       );
   }));

    model::based_native_function->attrs.insert("__str__", model::create_nfunc([](model::Object* self, model::List* args){
       auto self_nfn = dynamic_cast<model::NativeFunction*>(self);
       return model::create_str(
        "<NativeFunction" +
            (self_nfn->name.empty()
            ? ""
            : ": path='" + self_nfn->name + "'"
            )
            + " at " + ptr_to_string(self_nfn) + ">"
       );
   }));

    builtins.insert("Int", model::based_int);
    builtins.insert("Bool", model::based_bool);
    builtins.insert("Decimal", model::based_decimal);
    builtins.insert("List", model::based_list);
    builtins.insert("Dict", model::based_dict);
    builtins.insert("Str", model::based_str);
    builtins.insert("Func", model::based_function);
    builtins.insert("NFunc", model::based_native_function);
    builtins.insert("__Nil", model::based_nil);
    builtins.insert("Error", model::based_error);
    builtins.insert("Module", model::based_module);
    builtins.insert("__StopIterSignal__", model::stop_iter_signal);
}
}
