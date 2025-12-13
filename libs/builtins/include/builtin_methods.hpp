#pragma once
#include <functional>

#include "models.hpp"

namespace model {

// Int 类型原生函数
Object* int_add(Object* self, const List* args);
Object* int_sub(Object* self, const List* args);
Object* int_mul(Object* self, const List* args);
Object* int_div(Object* self, const List* args);
Object* int_pow(Object* self, const List* args);
Object* int_mod(Object* self, const List* args);
Object* int_eq(Object* self, const List* args);
Object* int_lt(Object* self, const List* args);
Object* int_gt(Object* self, const List* args);
//Object* int_str(Object* self, const List* args);
//Object* int_repr(Object* self, const List* args);

// Rational 类型原生函数
Object* rational_add(Object* self, const List* args);
Object* rational_sub(Object* self, const List* args);
Object* rational_mul(Object* self, const List* args);
Object* rational_div(Object* self, const List* args);
//Object* rational_pow(Object* self, const List* args);
//Object* rational_mod(Object* self, const List* args);
Object* rational_eq(Object* self, const List* args);
Object* rational_lt(Object* self, const List* args);
Object* rational_gt(Object* self, const List* args);
//Object* rational_str(Object* self, const List* args);
//Object* rational_repr(Object* self, const List* args);

// Nil 类型原生函数
Object* nil_eq(Object* self, const List* args);
//Object* nil_str(Object* self, const List* args);
//Object* nil_repr(Object* self, const List* args);

// Bool 类型原生函数
Object* bool_eq(Object* self, const List* args);
//Object* bool_str(Object* self, const List* args);
//Object* bool_repr(Object* self, const List* args);

// String 类型原生函数
Object* str_eq(Object* self, const List* args);
Object* str_add(Object* self, const List* args);
Object* str_mul(Object* self, const List* args);
Object* str_contains(Object* self, const List* args);
//Object* str_str(Object* self, const List* args);
//Object* str_repr(Object* self, const List* args);

// Dict 类型原生函数
Object* dict_eq(Object* self, const List* args);
Object* dict_add(Object* self, const List* args);
Object* dict_contains(Object* self, const List* args);
//Object* dict_str(Object* self, const List* args);
//Object* dict_repr(Object* self, const List* args);

// List 类型原生函数
Object* list_eq(Object* self, const List* args);
Object* list_add(Object* self, const List* args);
Object* list_mul(Object* self, const List* args);
Object* list_contains(Object* self, const List* args);
Object* list_append(Object* self, const List* args);
//Object* list_str(Object* self, const List* args);
//Object* list_repr(Object* self, const List* args);

}
