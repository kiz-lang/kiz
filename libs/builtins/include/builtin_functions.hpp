#include "models.hpp"

namespace builtin {

using NativeFuncPtr = std::function<model::Object*(model::Object*, const model::List*)>;

inline NativeFuncPtr print;

inline NativeFuncPtr input;

inline NativeFuncPtr isinstance;

inline NativeFuncPtr help;

inline NativeFuncPtr breakpointer;

inline NativeFuncPtr range;

inline NativeFuncPtr cmd;

inline NativeFuncPtr now;

inline NativeFuncPtr setattr;

inline NativeFuncPtr getattr;

inline NativeFuncPtr delattr;

inline NativeFuncPtr getrefc;

inline NativeFuncPtr copy;

inline NativeFuncPtr create;

inline NativeFuncPtr typeof;

}