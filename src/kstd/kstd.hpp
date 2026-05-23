//|    The kizc & kizrt's based standard library (kstd)
//|    author: azhz<azhz1107cat@outlook.com>
//|
//|     _        _      _ 
//|    | | _____| |_ __| |
//|    | |/ / __| __/ _` |
//|    |   <\__ \ || (_| |
//|    |_|\_\___/\__\__,_|
//|


#pragma once

// ===--------
// Useful Macros
// ===---------

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

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <limits.h>

// ===--------
// Kstd::functions
// ===---------


namespace kstd {

// ===--------
// Pointer POD
// ===---------
template <typename T>
struct Ptr {
    T* ptr;

    /// 显式构造，禁止隐式转换
    explicit constexpr Ptr(T* p = nullptr) noexcept : ptr(p) {}

    /// 解引用
    constexpr auto operator*() noexcept -> T& {
        return *ptr;
    }
    constexpr auto operator*() const noexcept -> const T& {
        return *ptr;
    }

    /// 成员访问箭头
    constexpr auto operator->() noexcept -> T* {
        return ptr;
    }
    constexpr auto operator->() const noexcept -> const T* {
        return ptr;
    }

    /// 下标访问
    constexpr auto operator[](ptrdiff_t off) noexcept -> T& {
        return ptr[off];
    }
    constexpr auto operator[](ptrdiff_t off) const noexcept -> const T& {
        return ptr[off];
    }

    /// 前置自增、自减
    constexpr auto operator++() noexcept -> Ptr& {
        ++ptr;
        return *this;
    }
    constexpr auto operator--() noexcept -> Ptr& {
        --ptr;
        return *this;
    }

    /// 加减偏移
    constexpr auto operator+(ptrdiff_t off) const noexcept -> Ptr {
        return Ptr{ptr + off};
    }
    constexpr auto operator-(ptrdiff_t off) const noexcept -> Ptr {
        return Ptr{ptr - off};
    }

    /// 指针差值
    constexpr auto operator-(Ptr other) const noexcept -> ptrdiff_t {
        return ptr - other.ptr;
    }

    /// 比较运算符
    constexpr auto operator==(Ptr other) const noexcept -> bool {
        return ptr == other.ptr;
    }
    constexpr auto operator!=(Ptr other) const noexcept -> bool {
        return ptr != other.ptr;
    }
    constexpr auto operator<(Ptr other) const noexcept -> bool {
        return ptr < other.ptr;
    }
    constexpr auto operator>(Ptr other) const noexcept -> bool {
        return ptr > other.ptr;
    }

    /// 转原生裸指针
    constexpr auto get() noexcept -> T* {
        return ptr;
    }
    constexpr auto get() const noexcept -> const T* {
        return ptr;
    }
    
    /// 取值解引用
    constexpr auto deref() const noexcept -> T {
        return *ptr;
    }

    /// 判空
    constexpr auto is_null() const noexcept -> bool {
        return ptr == nullptr;
    }
};

// ===--------
// Str POD
// ===---------

// ===--------
// Vec POD
// ===---------

// ===--------
// Option POD
// ===---------

// ===--------
// Result POD
// ===---------

// ===--------
// Memory Tools
// ===---------

namespace mem {
template<typename T>
[[nodiscard]] 
auto alloc(size_t count = 1) noexcept -> Ptr<T> {
    return Ptr(reinterpret_cast<T*>(malloc(sizeof(T) * count)));
}

template<typename T>
auto free(T* obj) noexcept -> void { ::free(obj); }

template<typename T>
[[nodiscard]]
auto realloc(T* obj, size_t count) noexcept -> Ptr<T>{
    return Ptr(reinterpret_cast<T*>(::realloc(obj, sizeof(T) * count)));
}
}

// ===--------
// Shell Tools
// ===---------

namespace shell {
namespace detail {
static void format_one(char* buf, size_t& off, size_t cap, int v) noexcept {
    off += snprintf(buf + off, cap - off, "%d", v);
}
static void format_one(char* buf, size_t& off, size_t cap, uint32_t v) noexcept {
    off += snprintf(buf + off, cap - off, "%u", v);
}
static void format_one(char* buf, size_t& off, size_t cap, const char* s) noexcept {
    if (!s) return;
    off += snprintf(buf + off, cap - off, "%s", s);
}
static void format_one(char* buf, size_t& off, size_t cap, const uint8_t* s) noexcept {
    format_one(buf, off, cap, reinterpret_cast<const char*>(s));
}
static void format_one(char* buf, size_t& off, size_t cap, const Str& s) noexcept {
    if (s.ptr.raw) {
        size_t l = s.len;
        size_t w = (l < cap - off) ? l : (cap - off - 1);
        memcpy(buf + off, s.ptr.raw, w);
        off += w;
    }
}

template<typename... Args>
static void format_impl(char* buf, size_t cap, const char* fmt, Args&&... args) noexcept {
    size_t off = 0;
    auto proc = [&](auto&& arg) noexcept {
        while (*fmt && off < cap - 1) {
            if (*fmt == '{' && *(fmt + 1) == '}') {
                fmt += 2;
                format_one(buf, off, cap, arg);
                break;
            }
            buf[off++] = *fmt++;
        }
    };
    (proc(args), ...);
    while (*fmt && off < cap - 1) buf[off++] = *fmt++;
    buf[off] = '\0';
}
constexpr size_t PRINT_BUF_CAP = 4096;
}

template<typename... Args>
auto println(const char* fmt, Args&&... args) noexcept {
    char buf[detail::PRINT_BUF_CAP];
    detail::format_impl(buf, detail::PRINT_BUF_CAP, fmt, args...);
    fputs(buf, stdout);
    fputc('\n', stdout);
}
template<typename... Args>
auto print_withoutendl(const char* fmt, Args&&... args) noexcept {
    char buf[detail::PRINT_BUF_CAP];
    detail::format_impl(buf, detail::PRINT_BUF_CAP, fmt, args...);
    fputs(buf, stdout);
}
inline auto flush() noexcept { fflush(stdout); }
}

// ===--------
// File System Tools
// ===---------

namespace fs {
inline auto read_as_str(const char* path) noexcept -> Str {
    if (!path) return {};
    FILE* f = fopen(path, "rb");
    if (!f) return {};
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return {}; }
    uint32_t len = static_cast<uint32_t>(sz);
    auto* p = mem::alloc<uint8_t>(len);
    if (!p) { fclose(f); return {}; }
    fread(p, 1, len, f);
    fclose(f);
    return Str(Ptr<uint8_t>(p), len);
}

static auto normalize_segment(char* out, size_t cap, const char* s) noexcept {
    size_t o = 0;
    while (*s && o < cap - 1) {
        if (*s == '/') {
            // 连续斜杠跳过
            while (*s == '/') s++;
            if (o > 0) out[o++] = '/';
        } else if (s[0] == '.' && s[1] == '/') {
            // ./ 跳过
            s += 2;
        } else if (s[0] == '.' && s[1] == '.' && (s[2] == '/' || s[2] == '\0')) {
            // ../ 回退
            s += (s[2] == '/') ? 3 : 2;
            if (o > 0) {
                o--;
                while (o > 0 && out[o-1] != '/') o--;
            }
        } else {
            out[o++] = *s++;
        }
    }
    out[o] = '\0';
}

// 支持 a + ../../y + b 这种，自动规范化
template<typename... Paths>
inline auto pathcat(char* out, size_t cap, Paths&&... paths) noexcept {
    out[0] = '\0';
    size_t off = 0;
    char seg[PATH_MAX];

    auto add = [&](const char* p) noexcept {
        if (!p || !*p) return;

        // 绝对路径直接重置
        if (p[0] == '/') {
            off = 0;
            out[off++] = '/';
            p++;
        } else if (off > 0 && out[off-1] != '/') {
            if (off < cap - 1) out[off++] = '/';
        }

        normalize_segment(seg, sizeof(seg), p);
        size_t sl = strlen(seg);
        if (sl == 0) return;

        if (off + sl >= cap) sl = cap - off - 1;
        memcpy(out + off, seg, sl);
        off += sl;
    };

    (add(paths), ...);
    out[off] = '\0';
}

inline auto filename(char* out, size_t cap, const char* path) noexcept {
    if (!path) { out[0] = '\0'; return; }
    const char* p = strrchr(path, '/');
    const char* fn = p ? (p + 1) : path;
    strncpy(out, fn, cap - 1);
    out[cap - 1] = '\0';
}

inline auto dirpath(char* out, size_t cap, const char* path) noexcept {
    if (!path) { out[0] = '\0'; return; }
    strncpy(out, path, cap - 1);
    out[cap - 1] = '\0';
    char* p = strrchr(out, '/');
    if (p && p != out) *p = '\0';
    else strcpy(out, ".");
}

inline auto is_abspath(const char* path) noexcept -> bool {
    return path && path[0] == '/';
}
}

}