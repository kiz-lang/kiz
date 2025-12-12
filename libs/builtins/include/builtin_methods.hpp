namespace model {

using NativeFuncPtr = std::function<Object*(Object*, const List*)>;

inline NativeFuncPtr int_add;
inline NativeFuncPtr int_sub;
inline NativeFuncPtr int_mul;
inline NativeFuncPtr int_div;
inline NativeFuncPtr int_pow;
inline NativeFuncPtr int_eq;
inline NativeFuncPtr int_lt;
inline NativeFuncPtr int_gt;
inline NativeFuncPtr int_str;
inline NativeFuncPtr int_repr;

inline NativeFuncPtr rational_add;
inline NativeFuncPtr rational_sub;
inline NativeFuncPtr rational_mul;
inline NativeFuncPtr rational_div;
inline NativeFuncPtr rational_pow;
inline NativeFuncPtr rational_eq;
inline NativeFuncPtr rational_lt;
inline NativeFuncPtr rational_gt;
inline NativeFuncPtr rational_str;
inline NativeFuncPtr rational_repr;

inline NativeFuncPtr nil_eq;
inline NativeFuncPtr nil_str;
inline NativeFuncPtr nil_repr;

inline NativeFuncPtr bool_eq;
inline NativeFuncPtr bool_str;
inline NativeFuncPtr bool_repr;

inline NativeFuncPtr str_eq;
inline NativeFuncPtr str_add;
inline NativeFuncPtr str_contains;
inline NativeFuncPtr str_str;
inline NativeFuncPtr str_repr;

inline NativeFuncPtr dict_eq;
inline NativeFuncPtr dict_add;
inline NativeFuncPtr dict_contains;
inline NativeFuncPtr dict_str;
inline NativeFuncPtr dict_repr;

inline NativeFuncPtr list_eq;
inline NativeFuncPtr list_add;
inline NativeFuncPtr list_contains;
inline NativeFuncPtr list_str;
inline NativeFuncPtr list_repr;


}