#pragma once

//|    The kizc & kizrt's based standard library (kstd)
//|    author: azhz<azhz1107cat@outlook.com>
//|
//|     _        _      _ 
//|    | | _____| |_ __| |
//|    | |/ / __| __/ _` |
//|    |   <\__ \ || (_| |
//|    |_|\_\___/\__\__,_|
//|

#define $VecForeach(i,v) for (auto i = 0; i < v.size(); ++i)
#ifdef _MSC_VER
#   define $Likely(x) (x)
#   define $Unlikely(x) (x)
#else
#   define $Likely(x)   __builtin_expect(!!(x), 1)
#   define $Unlikely(x) __builtin_expect(!!(x), 0)
#endif

#define $Assert(expr) \
do { \
    if (!(expr)) { \
        printf("Assert failed: %s\n", #expr); \
        __builtin_trap(); \
    } \
} while (0)

#define $AssertEq(lhs, rhs) \
do { \
    auto&& _a = (lhs); \
    auto&& _b = (rhs); \
    if (!(_a == _b)) { \
        printf("Assert equal failed: %s == %s\n", #lhs, #rhs); \
        __builtin_trap(); \
    } \
} while(0)

#define $Unimpl() \
do { \
    printf("Unimplemented code reached\n"); \
    __builtin_trap(); \
} while (0)

#define $Unreach() \
do { \
    printf("Unreachable code executed\n"); \
    __builtin_trap(); \
} while (0)

#define $IsWin  defined(_WIN32) || defined(_WIN64)
#define $IsLinux defined(__linux__)
#define $IsMac  defined(__APPLE__)

#define $IsX64  defined(__x86_64__) || defined(_M_X64)
#define $IsArm64 defined(__aarch64__) || defined(_M_ARM64)


namespace kstd {

#include "ptr.hpp"
#include "str.hpp"
#include "vec.hpp"
#include "option.hpp"
#include "result.hpp"
using uint32 = uint32_t;
using int32 = int32_t;
using uint8 = uint8_t;
}