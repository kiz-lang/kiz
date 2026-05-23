//|
//|     _        _      _ 
//|    | | _____| |_ __| |
//|    | |/ / __| __/ _` |
//|    |   <\__ \ || (_| |
//|    |_|\_\___/\__\__,_|
//|
//|    A simple standard library for some useful tools.
//|
//|    Only require cstdlib !
//|
//|    author: azhz<azhz1107cat@outlook.com>

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
// Kstd's POD and Function
// ===---------

namespace kstd {

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
struct Str {
    uint32_t len;
    Ptr<u8>  ptr;

    /// 空字符串构造
    constexpr Str() noexcept : len(0), ptr(nullptr) {}

    /// 显式构造（ptr 需指向合法内存，外部保证生命周期）
    explicit constexpr Str(Ptr<u8> p, uint32_t l) noexcept : len(l), ptr(p) {}

    /// 从 C 字符串构造（自动计算长度，不含 '\0'）
    explicit Str(const char* cstr) noexcept : len(0), ptr(nullptr) {
        if (cstr != nullptr) {
            len = static_cast<uint32_t>(strlen(cstr));
            ptr = Ptr<u8>(reinterpret_cast<u8*>(const_cast<char*>(cstr)));
        }
    }

    /// 获取指定下标字节
    constexpr auto operator[](uint32_t idx) noexcept -> u8& {
        assert(idx < len && "Str: index out of range");
        return ptr[idx];
    }
    constexpr auto operator[](uint32_t idx) const noexcept -> const u8& {
        assert(idx < len && "Str: index out of range");
        return ptr[idx];
    }

    /// 转 C 字符串（需外部保证 ptr 指向的内存以 '\0' 结尾，否则仅用于临时访问）
    constexpr auto c_str() const noexcept -> const char* {
        return reinterpret_cast<const char*>(ptr.get());
    }

    /// 获取子串（从 start 开始，取 len 个字节，不拷贝内存，仅指向原区域）
    constexpr auto substr(uint32_t start, uint32_t sub_len) const noexcept -> Str {
        assert(start + sub_len <= len && "Str: substr out of range");
        return Str(ptr + start, sub_len);
    }

    /// 字符串相等比较
    constexpr auto operator==(const Str& other) const noexcept -> bool {
        if (len != other.len) return false;
        return memcmp(ptr.get(), other.ptr.get(), len) == 0;
    }

    /// 字符串不等比较
    constexpr auto operator!=(const Str& other) const noexcept -> bool {
        return !(*this == other);
    }

    /// 字典序比较
    constexpr auto operator<(const Str& other) const noexcept -> bool {
        const uint32_t min_len = len < other.len ? len : other.len;
        const int cmp = memcmp(ptr.get(), other.ptr.get(), min_len);
        return cmp != 0 ? cmp < 0 : len < other.len;
    }

    /// 判空
    constexpr auto is_empty() const noexcept -> bool {
        return len == 0;
    }

    /// 检查是否包含指定字符
    constexpr auto contains(u8 ch) const noexcept -> bool {
        for (uint32_t i = 0; i < len; ++i) {
            if (ptr[i] == ch) return true;
        }
        return false;
    }

    /// 计算哈希值
    constexpr auto hash() const noexcept -> uint64_t {
        constexpr uint64_t fnv_offset = 14695981039346656037ULL;
        constexpr uint64_t fnv_prime = 1099511628211ULL;
        uint64_t h = fnv_offset;
        for (uint32_t i = 0; i < len; ++i) {
            h ^= static_cast<uint64_t>(ptr[i]);
            h *= fnv_prime;
        }
        return h;
    }
};

// ===--------
// Vec POD
// ===---------
template <typename T>
struct Vec {
    Ptr<T>  data;
    uint32_t len;
    uint32_t cap;

    /// 空构造
    constexpr Vec() noexcept : data(nullptr), len(0), cap(0) {}

    /// 从外部已分配内存绑定
    constexpr Vec(Ptr<T> buf, uint32_t buf_cap) noexcept
        : data(buf), len(0), cap(buf_cap)
    {}

    template <typename... Args>
    constexpr Vec(Args&&... args) noexcept
    : data(nullptr), len(0), cap(0) {
        constexpr size_t arg_cnt = sizeof...(Args);
        if constexpr (arg_cnt > 0) {
            reserve_maybe_realloc(static_cast<uint32_t>(arg_cnt));
            (push_maybe_realloc(args), ...);
        }
    }

    constexpr auto reserve_maybe_realloc(uint32_t new_cap) noexcept -> void {
        if (new_cap <= cap) return;
        auto old_ptr = data.get();
        auto new_ptr = mem::realloc(old_ptr, static_cast<size_t>(new_cap));
        if (!new_ptr) return;

        data = new_ptr;
        cap = new_cap;
    }

    constexpr auto push_maybe_realloc(const T& val) noexcept -> void {
        if (len >= cap) {
            uint32_t new_cap = (cap == 0) ? 4 : cap * 2;
            reserve_maybe_realloc(new_cap);
        }
        if (len < cap) {
            data[len++] = val;
        }
    }

    template<typename... Args>
    constexpr auto emplace_maybe_realloc(Args&&... args) noexcept -> T& {
        if (len >= cap) {
            uint32_t new_cap = cap == 0 ? 4 : cap * 2;
            reserve_maybe_realloc(new_cap);
        }
        T& ref = data[len++];
        new (&ref) T(args...);
        return ref;
    }

    constexpr auto extend_maybe_realloc(const T* src, uint32_t count) noexcept -> void {
        if (count == 0) return;
        uint32_t need = len + count;
        if (need > cap) {
            uint32_t new_cap = cap;
            while (new_cap < need)
                new_cap = new_cap == 0 ? 4 : new_cap * 2;
            reserve_maybe_realloc(new_cap);
        }
        memcpy(data.get() + len, src, sizeof(T) * count);
        len += count;
    }

    constexpr auto extend_maybe_realloc(const Vec<T>& other) noexcept -> void {
        extend_maybe_realloc(other.data.get(), other.len);
    }

    /// 判空
    constexpr auto isempty() const noexcept -> bool {
        return len == 0;
    }

    /// 预留容量
    constexpr auto reserve(uint32_t new_cap) noexcept -> void {
        if (new_cap > cap) {
            cap = new_cap;
        }
    }

    // 尾插，外部保证 cap 足够
    constexpr auto push(const T& val) noexcept -> void {
        if (len < cap) {
            data[len++] = val;
        }
    }

    /// 弹出末尾
    constexpr auto pop() noexcept -> void {
        if (len > 0) {
            len--;
        }
    }

    /// 清空长度（不释放内存）
    constexpr auto clear() noexcept -> void {
        len = 0;
    }

    /// 下标读写
    constexpr auto operator[](uint32_t idx) noexcept -> T& {
        $Assert(0 <= idx && idx < len);
        return data[idx];
    }
    constexpr auto operator[](uint32_t idx) const noexcept -> const T& {
        $Assert(0 <= idx && idx < len);
        return data[idx];
    }

    /// 首尾元素
    constexpr auto front() noexcept -> T& {
        return data[0];
    }
    constexpr auto front() const noexcept -> const T& {
        return data[0];
    }

    constexpr auto back() noexcept -> T& {
        return data[len - 1];
    }
    constexpr auto back() const noexcept -> const T& {
        return data[len - 1];
    }

    /// 获取长度、容量
    constexpr auto size() const noexcept -> uint32_t {
        return len;
    }
    constexpr auto capacity() const noexcept -> uint32_t {
        return cap;
    }

    /// 获取底层指针
    constexpr auto get_data() noexcept -> Ptr<T> {
        return data;
    }
    constexpr auto get_data() const noexcept -> Ptr<T> {
        return data;
    }

    /// 重置绑定外部内存
    constexpr auto reset(Ptr<T> buf, uint32_t buf_cap) noexcept -> void {
        data = buf;
        len = 0;
        cap = buf_cap;
    }
};

// ===--------
// Option POD
// ===---------
enum class OptTag : uint8_t {
    None,
    Some
};

template <typename T>
struct Option {
    OptTag tag;

    union {
        T val;
    } as;

    /// 构造空值
    static constexpr auto none() noexcept -> Option {
        Option o{};
        o.tag = OptTag::None;
        return o;
    }

    /// 构造有值
    static constexpr auto some(const T& v) noexcept -> Option {
        Option o{};
        o.tag = OptTag::Some;
        o.as.val = v;
        return o;
    }

    /// 判断
    constexpr auto is_none() const noexcept -> bool {
        return tag == OptTag::None;
    }

    constexpr auto is_some() const noexcept -> bool {
        return tag == OptTag::Some;
    }

    /// 取值（不检查，使用者保证有值）
    constexpr auto unwrap() noexcept -> T& {
        assert(is_some() && "Option: unwrap on none");
        return as.val;
    }

    constexpr auto unwrap() const noexcept -> const T& {
        assert(is_some() && "Option: unwrap on none");
        return as.val;
    }

    /// 有值返回自身，无值给默认值
    constexpr auto unwrap_or(const T& def) const noexcept -> T {
        return is_some() ? as.val : def;
    }

    /// 丢弃值，转为空
    constexpr auto take() noexcept -> void {
        tag = OptTag::None;
    }
};

// ===--------
// Result POD
// ===---------
enum class ResTag : uint8_t {
    Ok,
    Err
};

template <typename T, typename E>
struct Result {
    ResTag tag;

    union {
        T ok;
        E err;
    } as;

    /// 构造 Ok
    static constexpr auto ok(const T& val) noexcept -> Result {
        Result r{};
        r.tag = ResTag::Ok;
        r.as.ok = val;
        return r;
    }

    /// 构造 Err
    static constexpr auto err(const E& val) noexcept -> Result {
        Result r{};
        r.tag = ResTag::Err;
        r.as.err = val;
        return r;
    }

    /// 判断
    constexpr auto is_ok() const noexcept -> bool {
        return tag == ResTag::Ok;
    }

    constexpr auto is_err() const noexcept -> bool {
        return tag == ResTag::Err;
    }

    /// 取值 不做检查 使用者自己保证合法
    constexpr auto unwrap_ok() noexcept -> T& {
        return as.ok;
    }
    constexpr auto unwrap_ok() const noexcept -> const T& {
        return as.ok;
    }

    constexpr auto unwrap_err() noexcept -> E& {
        return as.err;
    }
    constexpr auto unwrap_err() const noexcept -> const E& {
        return as.err;
    }

    /// 兜底取值，出错给默认值
    constexpr auto unwrap_or(const T& def) const noexcept -> T {
        return is_ok() ? as.ok : def;
    }
};


}