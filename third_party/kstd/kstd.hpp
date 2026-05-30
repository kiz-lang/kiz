//|
//|     _        _      _ 
//|    | | _____| |_ __| |
//|    | |/ / __| __/ _` |
//|    |   <\__ \ || (_| |
//|    |_|\_\___/\__\__,_|
//|
//|
//|    kiz 简易标准库 (kstd)
//|
//|    面向实用工具开发的轻量级基础库。
//|    整体仅依赖 C 标准库，不引入任何第三方依赖，可在各类环境中稳定部署与使用。
//|
//|    核心类型：
//|    - Ptr<T>：原生指针包装器，封装指针常用操作，降低野指针带来的风险。
//|    - Str：字符串切片类型，代表只读字符串片段，不持有内存所有权。
//|    - MutStr：可变字符串，支持内容修改、追加与动态扩容。
//|    - Slice：通用切片类型，用于描述动态数组的连续内存区域。
//|    - Vec<T>：动态数组，支持双模式内存管理，兼具自动扩容与手动内存控制能力。
//|    - MemPool<T>：通用内存池，采用块式内存分配策略，有效降低内存碎片。
//|    - HashMap<ValueT>：开放寻址哈希表，高性能键值存储容器。
//|    - Option<T>：空安全可选值类型，显式区分有效数据与空值状态。
//|    - Result<T, E>：错误处理结果类型，统一正常返回与异常反馈逻辑。
//|    - NoCopyMove：禁止拷贝与移动构造，空基类。
//|
//|    核心函数：
//|    - move<T>(T obj) -> T&&：左值转右值。
//|    - forward<T>(T&& obj) -> T&&：完美转发。
//|    - memcpy<T>(Ptr<T> dst, const Ptr<T> src, size_t count)：按元素个数拷贝内存。
//|    - alloc<T>(size_t count) -> Ptr<T>：为指定数量的 T 类型对象分配堆内存，
//|      返回包装后的指针。分配失败直接调用abort。
//|    - alloc_or<T>(size_t count) -> Option<Ptr<T>>：安全版内存分配接口，
//|      分配失败时返回空可选值。
//|    - realloc<T>(Ptr<T> obj, size_t count) -> Ptr<T>：调整已分配动态内存的大小，
//|      自动迁移原有数据至新内存空间，分配失败将直接调用abort。
//|    - realloc_or<T>(Ptr<T> obj, size_t count) -> Option<Ptr<T>>：安全版内存重分配接口，
//|      操作失败时原有内存保持有效，通过空可选值标识执行错误。
//|    - free<T>(Ptr<T> obj)：释放 Ptr 管理的动态内存，自动判空，避免重复释放与非法访问。
//|    - println(Str text)：将只读字符串切片输出至标准流，末尾自动添加换行符。
//|    - println(MutStr text)：将可变字符串内容输出至标准流，末尾自动添加换行符。
//|    - print(Str text)：将只读字符串切片输出至标准流，不追加换行，内容暂存于输出缓冲区。
//|    - print(MutStr text)：将可变字符串内容输出至标准流，不追加换行符。
//|    - shellflush()：强制刷新标准输出缓冲区，将缓存内容立即写入目标设备。
//|    - pathnormalize(Str path) -> Str：标准化文件路径，剔除多余分隔符与 .、.. 等相对路径节点，
//|      返回规整后的路径切片。
//|    - pathcat(MutStr path, ...)：拼接多段路径为完整路径，自动适配不同平台的路径分隔符。
//|    - base_filename(Str path) -> Str：从完整路径中提取文件名，剔除目录前缀。
//|    - is_abspath(Str path) -> bool：根据当前平台规则，判断传入路径是否为绝对路径。
//|    - readfile(Str path, Str out) -> KstdIOError：读取指定文件内容至输出缓冲区，
//|      通过错误枚举标识执行结果与各类读取异常。
//|    - writefile(Str path, Str text) -> KstdIOError：将字符串内容写入指定文件，支持新建与覆盖文件，
//|      通过错误枚举反馈运行状态。
//|    - enumeration_name<T>(T enum_item) -> Str：将枚举项转为字符串。
//|        
//|    核心宏：
//|    - $ForeachVec(var, vec)：动态数组遍历宏，依次将容器元素绑定至循环变量，简化迭代代码。
//|    - $Guard(expr)：守卫语句，后面需要接else。
//|    - $ReturnIfErr(var)：简化Result判断is_err()调用。
//|    - $Likely(expr)：分支预测优化宏，标记表达式大概率成立，引导编译器优化主流分支，提升运行效率。
//|    - $Unlikely(expr)：分支预测优化宏，标记表达式极少成立，对异常分支做专项优化，减少指令跳转开销。
//|    - $Assert(expr)：运行时断言宏，校验逻辑表达式有效性；调试模式下断言失败会主动弹出提示。
//|    - $AssertEq(left, right)：等值断言宏，校验两个运算结果是否完全相等，用于逻辑校验与简易单元检查。
//|    - $Unimplement()：标记未实现的代码分支，执行到该位置时主动提醒开发者补全功能。
//|    - $Unreachable()：标记理论上永远不会执行的代码分支，作为逻辑兜底，意外触发时抛出错误提示。
//|    - $IsWindows：编译期平台判断宏，仅在 Windows 系统下判定为真。
//|    - $IsLinux：编译期平台判断宏，仅在 Linux 系统下判定为真。
//|    - $IsMac：编译期平台判断宏，仅在 macOS 系统下判定为真。
//|    - $NoMacro：当前头文件定义的全部 $ 前缀宏统一 undef。
//|
//|    核心常量：
//|   - constInvalidIdx：非法索引值。
//|
//|    库特性：
//|        内部所有数据结构均采用标准 POD 设计，内存布局规整、占用体积极小，
//|        不使用RTTI和 C++ Exception
//|        适配嵌入式及资源受限的运行环境。容器组件支持外部手动内存管理、自动动态扩容两种工作模式，
//|        开发者可根据业务场景自由切换。整套 API 遵循现代 C++ 设计理念，语义清晰、调用逻辑统一。
//|        本库实现全平台、全架构兼容，可在主流操作系统与硬件架构上正常运行。
//|        采用经典单头文件 STB 风格分发，无需复杂编译与链接配置，能够快速集成至任意项目中。
//|
//|    作者: azhz<azhz1107cat@outlook.com>
//|


#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <new>

#if $IsWindows
#   include <winnt.h>
#   include <winbase.h>
#   undef near
#   undef far
#else
#   include <unistd.h>
#endif

namespace kstd {

// ===-----------------------
//
// 实用的宏定义
//
// ===-----------------------

/// 遍历支持下标访问与 size() 方法的容器
/// 示例:
///     $ForeachVec(index, peoples){
///         println(peoples[index]);
///     }
#define $ForeachVec(i,v) for (auto i = 0; i < v.size(); ++i)

/// 条件守卫语句
/// 示例:
///     $Guard(1==1) else {
///         return;
///     }
#define $Guard(expr) if (expr) {}

/// 结果为错误时直接返回
/// 示例:
///     auto a = fetch_data();
///     $ReturnIfErr(a);
///     auto a_val = a.must();
#define $ReturnIfErr(var) if (var.is_err()) { return var; }

/// 分支预测：标记分支为大概率执行
/// 示例:
///     if ($Likely(user.age < 100)) {
///         println(user.to_string());
///     }
#ifdef _MSC_VER
#   define $Likely(x) (x)
#else
#   define $Likely(x)   __builtin_expect(!!(x), 1)
#endif

/// 分支预测：标记分支为极少执行
/// 示例:
///     if ($Unlikely(user.age > 100)) {
///         println(user.to_string());
///     }
#ifdef _MSC_VER
#   define $Unlikely(x) (x)
#else
#   define $Unlikely(x) __builtin_expect(!!(x), 0)
#endif

/// 运行时断言，表达式为假则终止程序
/// 示例:
///     $Assert(ptr != nullptr);
#define $Assert(expr) \
do { \
    if (!(expr)) { \
        printf("Assert failed: %s\n", #expr); \
        __builtin_trap(); \
    } \
} while (0)

/// 等值断言，两个表达式不相等则终止程序
/// 示例:
///     $AssertEq(ret_code, 0);
#define $AssertEq(lhs, rhs) \
do { \
    auto&& _a = (lhs); \
    auto&& _b = (rhs); \
    if (!(_a == _b)) { \
        printf("Assert equal failed: %s == %s\n", #lhs, #rhs); \
        __builtin_trap(); \
    } \
} while (0)

/// 标记未实现代码分支，执行到此处则终止程序
/// 示例:
///     default:
///         $Unimplement();
#define $Unimplement() \
do { \
    printf("Unimplemented code reached\n"); \
    __builtin_trap(); \
} while (0)

/// 标记不可达代码分支，意外执行则终止程序
/// 示例:
///     while (true) { break; }
///     $Unreachable();
#define $Unreachable() \
do { \
    printf("Unreachable code executed\n"); \
    __builtin_trap(); \
} while (0)

/// 编译期判断：当前为 Windows 平台则为真
/// 示例:
///     #if $IsWindows
///         // Windows 平台专属代码
///     #endif
#define $IsWindows  defined(_WIN32) || defined(_WIN64)

/// 编译期判断：当前为 Linux 平台则为真
/// 示例:
///     #if $IsLinux
///         // Linux 平台专属代码
///     #endif
#define $IsLinux defined(__linux__)

/// 编译期判断：当前为 macOS 平台则为真
/// 示例:
///     #if $IsMac
///         // macOS 平台专属代码
///     #endif
#define $IsMac  defined(__APPLE__)


// ===-----------------------
//
// 核心常量
//
// ===-----------------------

/// 非法索引值
constexpr uint32_t constInvalidIndex = 0xFFFFFFFFU;

// 
// 不准拷贝与移动, 空基类
//
class NoCopyMove {
public:
    NoCopyMove() = default;
    ~NoCopyMove() = default;
    NoCopyMove(const NoCopyMove&) = delete;
    NoCopyMove(NoCopyMove&&) = delete;
    NoCopyMove& operator=(const NoCopyMove&) = delete;
    NoCopyMove& operator=(NoCopyMove&&) = delete;
};


// ===-----------------------
//
// Kstd's PODs
// 
// ===-----------------------

//
// 原始指针类型
// kstd所有函数不允许返回C++裸指针，要么返回引用，要么返回Ptr<T>类型
//

template <typename T>
struct Ptr {
    T* ptr;

    /// 显式构造，禁止隐式转换
    explicit constexpr Ptr(T* p = nullptr) noexcept : ptr(p) {}

    /// 获取内部裸指针（少数必要场景使用）
    constexpr auto get() noexcept -> T* {
        return ptr;
    }
    constexpr auto get() const noexcept -> const T* {
        return ptr;
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
    
    /// 取值解引用
    constexpr auto deref() const noexcept -> T {
        return *ptr;
    }

    /// 判空
    constexpr auto is_null() const noexcept -> bool {
        return ptr == nullptr;
    }
};

// 
// 字符串切片类型
// 
struct Str {
    uint32_t len;
    Ptr<uint8_t>  ptr;

    /// 空字符串构造
    constexpr Str() noexcept : len(0), ptr(nullptr) {}

    /// 显式构造（ptr 需指向合法内存，外部保证生命周期）
    explicit constexpr Str(Ptr<uint8_t> p, uint32_t l) noexcept : len(l), ptr(p) {}

    /// 从 C 字符串构造（自动计算长度，不含 '\0'）
    explicit Str(const char* cstr) noexcept : len(0), ptr(nullptr) {
        if (cstr != nullptr) {
            len = static_cast<uint32_t>(strlen(cstr));
            ptr = Ptr<uint8_t>(reinterpret_cast<uint8_t*>(const_cast<char*>(cstr)));
        }
    }

    /// 获取指定下标字节
    constexpr auto operator[](uint32_t idx) noexcept -> uint8_t& {
        assert(idx < len && "Str: index out of range");
        return ptr[idx];
    }
    constexpr auto operator[](uint32_t idx) const noexcept -> const uint8_t& {
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
    constexpr auto contains(uint8_t ch) const noexcept -> bool {
        for (uint32_t i = 0; i < len; ++i) {
            if (ptr[i] == ch) return true;
        }
        return false;
    }

    /// 计算哈希值 (FNV-1a 64bit)
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

// 
// 可变字符串类型
// 拥有内存所有权，自动管理缓冲区，保证末尾 '\0'
//
struct MutStr {
    Vec<uint8_t> _buf;   // 字符串缓冲区

    /// 空字符串构造
    constexpr MutStr() noexcept : _buf() {}

    /// 预分配容量构造
    constexpr MutStr(uint32_t prealloc) noexcept : _buf() {
        _buf.reserve(prealloc + 1);
        _buf.push_back(0);  // 初始 '\0'
    }

    /// 析构自动释放内存（由 Vec 管理）
    ~MutStr() = default;

    /// 禁止拷贝与移动（继承 Vec 的限制）
    MutStr(const MutStr&) = delete;
    MutStr(MutStr&&) = delete;
    MutStr& operator=(const MutStr&) = delete;
    MutStr& operator=(MutStr&&) = delete;

    /// 追加单个字符
    auto push_back(uint8_t c) noexcept -> void {
        // 去掉原来的末尾 '\0'，追加字符，再补 '\0'
        if (_buf.size() > 0) _buf.pop();
        _buf.push_back(c);
        _buf.push_back(0);
    }

    /// 追加字符串切片
    auto append(Str s) noexcept -> void {
        if (_buf.size() > 0) _buf.pop(); // 去 '\0'
        for (uint32_t i = 0; i < s.len; ++i) _buf.push_back(s[i]);
        _buf.push_back(0);
    }

    /// 追加 C 字符串
    auto append(const char* cstr) noexcept -> void {
        append(Str(cstr));
    }

    /// 清空字符串
    auto clear() noexcept -> void {
        _buf.clear();
        _buf.push_back(0);
    }

    /// 获取字符串长度（不含结尾 '\0'）
    auto size() const noexcept -> uint32_t {
        return _buf.size() > 0 ? _buf.size() - 1 : 0;
    }

    /// 获取 C 风格字符串指针
    auto c_str() const noexcept -> const char* {
        return _buf.isempty() ? "" : reinterpret_cast<const char*>(_buf.get_data().get());
    }

    /// 下标访问（可修改）
    auto operator[](uint32_t idx) noexcept -> uint8_t& {
        assert(idx < size() && "MutStr: index out of range");
        return _buf[idx];
    }
    auto operator[](uint32_t idx) const noexcept -> const uint8_t& {
        assert(idx < size() && "MutStr: index out of range");
        return _buf[idx];
    }

    /// 转为 Str 切片
    auto as_str() const noexcept -> Str {
        return Str(_buf.get_data(), size());
    }
};

//
// 动态数组类型
//
template <typename T>
struct Vec {
    Ptr<T>    data;
    uint32_t  len;
    uint32_t  cap;
    bool      external;   // 是否绑定外部缓冲区（外部不释放）

    /// 空构造（自动管理内存）
    constexpr Vec() noexcept : data(nullptr), len(0), cap(0), external(false) {}

    /// 绑定外部已分配内存（外部管理，不扩容）
    constexpr Vec(Ptr<T> buf, uint32_t buf_cap) noexcept
        : data(buf), len(0), cap(buf_cap), external(true)
    {}

    /// 析构自动释放内部内存
    ~Vec() noexcept {
        if (!external && !data.is_null()) {
            ::free(data.get());
        }
    }

    /// 禁止拷贝
    Vec(const Vec&) = delete;
    Vec& operator=(const Vec&) = delete;

    /// 移动构造
    Vec(Vec&& other) noexcept 
        : data(other.data), len(other.len), cap(other.cap), external(other.external) {
        other.data = Ptr<T>(nullptr);
        other.len = 0;
        other.cap = 0;
        other.external = false;
    }
    /// 移动赋值
    Vec& operator=(Vec&& other) noexcept {
        if (this != &other) {
            if (!external && !data.is_null()) ::free(data.get());
            data = other.data;
            len = other.len;
            cap = other.cap;
            external = other.external;
            other.data = Ptr<T>(nullptr);
            other.len = 0;
            other.cap = 0;
            other.external = false;
        }
        return *this;
    }

    /// 判空
    constexpr auto isempty() const noexcept -> bool {
        return len == 0;
    }

    /// 预留容量（自动模式分配内存，外部模式仅检查）
    auto reserve(uint32_t new_cap) noexcept -> void {
        if (new_cap <= cap) return;
        if (external) {
            // 外部缓冲区不可扩容，仅允许预留不超过现有容量
            assert(new_cap <= cap && "Vec: external buffer cannot be grown");
            return;
        }
        // 自动内存管理：重新分配
        size_t bytes = sizeof(T) * new_cap;
        T* new_data = static_cast<T*>(::realloc(data.get(), bytes));
        assert(new_data != nullptr && "Vec: reserve failed");
        data = Ptr<T>(new_data);
        cap = new_cap;
    }

    /// 追加元素
    auto push_back(const T& value) noexcept -> void {
        if (len >= cap) {
            if (external) {
                assert(false && "Vec: external buffer overflow");
                return;
            }
            // 自动扩容策略：2倍增长，最小容量4
            uint32_t new_cap = cap == 0 ? 4 : cap * 2;
            reserve(new_cap);
        }
        data[len] = value;
        ++len;
    }

    /// 弹出末尾元素
    auto pop() noexcept -> void {
        if (len > 0) {
            --len;
        }
    }

    /// 清空数组（保留容量）
    auto clear() noexcept -> void {
        len = 0;
    }

    /// 下标读写
    constexpr auto operator[](uint32_t idx) noexcept -> T& {
        $Assert(idx < len);
        return data[idx];
    }
    constexpr auto operator[](uint32_t idx) const noexcept -> const T& {
        $Assert(idx < len);
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
};


//
// 通用切片类型（非拥有）
//
template <typename T>
struct Slice {
    Ptr<T>    ptr;
    uint32_t  len;

    constexpr Slice() noexcept : ptr(nullptr), len(0) {}
    constexpr Slice(Ptr<T> p, uint32_t l) noexcept : ptr(p), len(l) {}

    /// 从 Vec 隐式构造
    Slice(const Vec<T>& v) noexcept : ptr(v.get_data()), len(v.size()) {}

    constexpr auto operator[](uint32_t idx) noexcept -> T& {
        assert(idx < len && "Slice: index out of range");
        return ptr[idx];
    }
    constexpr auto operator[](uint32_t idx) const noexcept -> const T& {
        assert(idx < len && "Slice: index out of range");
        return ptr[idx];
    }

    constexpr auto size() const noexcept -> uint32_t { return len; }
    constexpr auto is_empty() const noexcept -> bool { return len == 0; }
};


// 
// 可选类型
// 
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


// 
// 结果类型
// 
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


//
// 文件 I/O 错误枚举
//
enum class KstdIOError : uint8_t {
    None = 0,            // 成功
    FileNotFound,        // 文件不存在
    AccessDenied,        // 权限不足
    ReadError,           // 读取失败
    WriteError,          // 写入失败
    Unknown              // 未知错误
};


// ===-----------------------
//
// 内存池类型
// 采用固定块大小与空闲链表，仅适用于平凡类型
//
template <typename T>
struct MemPool {
    struct Block {
        alignas(T) uint8_t storage[sizeof(T)];
        Block* next;   // 空闲链表节点
    };

    Ptr<Block>  mem;        // 底层连续内存
    Block*      free_list;  // 空闲链表头
    uint32_t    total;      // 总块数
    uint32_t    avail;      // 剩余块数

    /// 构造内存池并预分配 block_cnt 个块
    MemPool(uint32_t block_cnt) noexcept
        : mem(nullptr), free_list(nullptr), total(block_cnt), avail(block_cnt)
    {
        if (block_cnt == 0) return;
        // 一次性分配连续内存
        size_t bytes = sizeof(Block) * block_cnt;
        Block* raw = static_cast<Block*>(::malloc(bytes));
        assert(raw != nullptr && "MemPool: allocation failed");
        mem = Ptr<Block>(raw);

        // 构建空闲链表
        free_list = raw;
        for (uint32_t i = 0; i < block_cnt - 1; ++i) {
            raw[i].next = &raw[i + 1];
        }
        raw[block_cnt - 1].next = nullptr;
    }

    /// 析构释放内存
    ~MemPool() noexcept {
        if (!mem.is_null()) {
            ::free(mem.get());
        }
    }

    /// 禁止拷贝
    MemPool(const MemPool&) = delete;
    MemPool& operator=(const MemPool&) = delete;

    /// 获取一个空闲块，返回指向该块中 T 类型对象的指针
    auto acquire() noexcept -> Ptr<T> {
        if (avail == 0) {
            assert(false && "MemPool: no free blocks");
            return Ptr<T>(nullptr);
        }
        Block* blk = free_list;
        free_list = blk->next;
        --avail;

        // 在已分配内存上构造默认对象（平凡类型无操作）
        T* obj = reinterpret_cast<T*>(blk->storage);
        // 这里假设 T 是平凡类型，不需要调用构造函数
        return Ptr<T>(obj);
    }

    /// 释放由 acquire 分配的块
    auto release(Ptr<T> obj) noexcept -> void {
        if (obj.is_null()) return;
        // 从对象指针回退到 Block 首地址（标准布局保证）
        Block* blk = reinterpret_cast<Block*>(reinterpret_cast<uint8_t*>(obj.get()) - offsetof(Block, storage));
        blk->next = free_list;
        free_list = blk;
        ++avail;
    }

    /// 剩余可用块数
    auto remaining() const noexcept -> uint32_t {
        return avail;
    }
};


// ===-----------------------
//
// 开放寻址哈希表
// Key 固定为 Str 切片（不拷贝键数据，要求键生命周期覆盖使用期）
// Value 为模板参数 ValueT
//
template <typename ValueT>
struct HashMap {
    enum class SlotState : uint8_t {
        Empty,
        Occupied,
        Deleted
    };

    struct Slot {
        Str       key;
        ValueT    value;
        SlotState state;
    };

    Vec<Slot>  slots;
    uint32_t   count;       // 实际元素数量
    uint32_t   limit;       // 扩容阈值 (capacity * 0.7)

    /// 初始构造
    HashMap() noexcept : slots(), count(0), limit(0) {
        _init_slots(8);
    }

    /// 禁止拷贝
    HashMap(const HashMap&) = delete;
    HashMap& operator=(const HashMap&) = delete;

    /// 查找键，返回指向值的指针（找不到返回空 Option）
    auto find(const Str& key) noexcept -> Option<Ptr<ValueT>> {
        const uint32_t cap = slots.capacity();
        if (cap == 0) return Option<Ptr<ValueT>>::none();

        uint64_t h = key.hash();
        uint32_t idx = static_cast<uint32_t>(h % cap);

        for (uint32_t i = 0; i < cap; ++i) {
            Slot& s = slots[idx];
            if (s.state == SlotState::Empty) {
                return Option<Ptr<ValueT>>::none();
            }
            if (s.state == SlotState::Occupied && s.key == key) {
                return Option<Ptr<ValueT>>::some(Ptr<ValueT>(&s.value));
            }
            // 线性探测
            idx = (idx + 1) % cap;
        }
        return Option<Ptr<ValueT>>::none();
    }

    /// 插入键值对，若键已存在则覆盖并返回旧值，否则插入并返回 none
    auto insert(const Str& key, const ValueT& value) noexcept -> Option<ValueT> {
        if (count >= limit) _resize(capacity() * 2);

        uint32_t cap = slots.capacity();
        uint64_t h = key.hash();
        uint32_t idx = static_cast<uint32_t>(h % cap);

        // 查找插入位置或重复键
        int32_t first_deleted = -1;
        for (uint32_t i = 0; i < cap; ++i) {
            Slot& s = slots[idx];
            if (s.state == SlotState::Empty) {
                // 优先使用删除位
                if (first_deleted >= 0) idx = static_cast<uint32_t>(first_deleted);
                Slot& target = slots[idx];
                target.key = key;
                target.value = value;
                target.state = SlotState::Occupied;
                ++count;
                return Option<ValueT>::none();
            }
            if (s.state == SlotState::Deleted) {
                if (first_deleted < 0) first_deleted = static_cast<int32_t>(idx);
            }
            else if (s.state == SlotState::Occupied && s.key == key) {
                // 键已存在，覆盖
                ValueT old = s.value;
                s.value = value;
                return Option<ValueT>::some(old);
            }
            idx = (idx + 1) % cap;
        }

        // 表满或逻辑错误
        assert(false && "HashMap: insert failed (table full?)");
        return Option<ValueT>::none();
    }

    /// 移除键，成功返回 true
    auto erase(const Str& key) noexcept -> bool {
        uint32_t cap = slots.capacity();
        if (cap == 0) return false;

        uint64_t h = key.hash();
        uint32_t idx = static_cast<uint32_t>(h % cap);

        for (uint32_t i = 0; i < cap; ++i) {
            Slot& s = slots[idx];
            if (s.state == SlotState::Empty) {
                return false;
            }
            if (s.state == SlotState::Occupied && s.key == key) {
                s.state = SlotState::Deleted;
                --count;
                return true;
            }
            idx = (idx + 1) % cap;
        }
        return false;
    }

    /// 清空哈希表
    auto clear() noexcept -> void {
        _init_slots(8);
        count = 0;
        limit = static_cast<uint32_t>(8 * 0.7);
    }

    /// 元素数量
    auto size() const noexcept -> uint32_t { return count; }

    /// 当前容量
    auto capacity() const noexcept -> uint32_t { return slots.capacity(); }

private:
    // 初始化槽位数组
    auto _init_slots(uint32_t cap) noexcept -> void {
        slots = Vec<Slot>();
        slots.reserve(cap);
        for (uint32_t i = 0; i < cap; ++i) {
            Slot s{};
            s.state = SlotState::Empty;
            slots.push_back(s);
        }
    }

    // 重新哈希扩容
    auto _resize(uint32_t new_cap) noexcept -> void {
        if (new_cap < 8) new_cap = 8;
        // 备份旧数据
        Vec<Slot> old_slots = ::move(slots);
        uint32_t old_cap = old_slots.capacity();

        // 初始化新槽位
        _init_slots(new_cap);
        limit = static_cast<uint32_t>(new_cap * 0.7);

        // 重新插入旧元素
        for (uint32_t i = 0; i < old_cap; ++i) {
            Slot& old = old_slots[i];
            if (old.state == SlotState::Occupied) {
                // 直接插入（不检查重复，因为旧表无重复）
                uint64_t h = old.key.hash();
                uint32_t idx = static_cast<uint32_t>(h % new_cap);
                while (slots[idx].state == SlotState::Occupied) {
                    idx = (idx + 1) % new_cap;
                }
                slots[idx] = old;
                ++count;  // 注意 count 已在 _init_slots 中隐含重置为0
            }
        }
        // old_slots 析构释放内存
    }
};


// ===-----------------------
//
// 全局工具函数
//
// ===-----------------------

/// 左值转右值
template <typename T>
constexpr auto move(T&& obj) noexcept -> T&& {
    return static_cast<T&&>(obj);
}

/// 完美转发
template <typename T>
constexpr auto forward(T&& obj) noexcept -> T&& {
    return static_cast<T&&>(obj);
}

/// 按元素个数拷贝内存（内部使用 memmove）
template <typename T>
auto memcpy(Ptr<T> dst, const Ptr<T> src, size_t count) noexcept -> void {
    if (count == 0) return;
    ::memmove(dst.get(), src.get(), count * sizeof(T));
}

/// 分配堆内存（失败直接 abort）
template <typename T>
auto alloc(size_t count) noexcept -> Ptr<T> {
    void* p = ::malloc(sizeof(T) * count);
    if ($Unlikely(p == nullptr)) {
        ::abort();
    }
    return Ptr<T>(static_cast<T*>(p));
}

/// 安全版内存分配（失败返回空 Option）
template <typename T>
auto alloc_or(size_t count) noexcept -> Option<Ptr<T>> {
    void* p = ::malloc(sizeof(T) * count);
    if (p == nullptr) {
        return Option<Ptr<T>>::none();
    }
    return Option<Ptr<T>>::some(Ptr<T>(static_cast<T*>(p)));
}

/// 调整已分配内存大小（失败 abort）
template <typename T>
auto realloc(Ptr<T> obj, size_t count) noexcept -> Ptr<T> {
    void* p = ::realloc(obj.get(), sizeof(T) * count);
    if ($Unlikely(p == nullptr)) {
        ::abort();
    }
    return Ptr<T>(static_cast<T*>(p));
}

/// 安全版内存重分配（失败返回空 Option，原内存保持有效）
template <typename T>
auto realloc_or(Ptr<T> obj, size_t count) noexcept -> Option<Ptr<T>> {
    void* p = ::realloc(obj.get(), sizeof(T) * count);
    if (p == nullptr) {
        return Option<Ptr<T>>::none();
    }
    return Option<Ptr<T>>::some(Ptr<T>(static_cast<T*>(p)));
}

/// 释放动态内存
template <typename T>
auto free(Ptr<T> obj) noexcept -> void {
    if (!obj.is_null()) {
        ::free(obj.get());
    }
}

/// 输出 Str 并换行
inline auto println(Str text) noexcept -> void {
    if (text.len > 0) {
        ::fwrite(text.ptr.get(), 1, text.len, stdout);
    }
    ::fputc('\n', stdout);
}

/// 输出 MutStr 并换行
inline auto println(const MutStr& text) noexcept -> void {
    println(text.as_str());
}

/// 输出 Str（不追加换行）
inline auto print(Str text) noexcept -> void {
    if (text.len > 0) {
        ::fwrite(text.ptr.get(), 1, text.len, stdout);
    }
}

/// 输出 MutStr（不追加换行）
inline auto print(const MutStr& text) noexcept -> void {
    print(text.as_str());
}

/// 刷新标准输出缓冲区
inline auto shellflush() noexcept -> void {
    ::fflush(stdout);
}

/// 标准化文件路径（使用内部静态缓冲区，非线程安全）
inline auto pathnormalize(Str path) noexcept -> Str {
    static char buf[4096];  // 临时缓冲区
    uint32_t j = 0;
    bool is_abs = is_abspath(path);

    // 平台分隔符
    const char sep = 
#if $IsWindows
        '\\';
#else
        '/';
#endif

    // 简化实现：去除重复分隔符，处理 '.' 和 '..'（基础）
    for (uint32_t i = 0; i < path.len; ++i) {
        uint8_t c = path[i];
        if (c == '/' || c == '\\') {
            // 统一为平台分隔符
            if (j > 0 && buf[j-1] != sep) {
                buf[j++] = sep;
            }
        } else if (c == '.' && (i+1 < path.len && (path[i+1] == '/' || path[i+1] == '\\' || i+1 == path.len))) {
            // 跳过 '.' 自身
            continue;
        } else if (c == '.' && i+1 < path.len && path[i+1] == '.' && 
                   (i+2 == path.len || path[i+2] == '/' || path[i+2] == '\\')) {
            // ".." 回退一级
            if (j > 0 && buf[j-1] == sep) --j; // 去掉末尾分隔符
            while (j > 0 && buf[j-1] != sep) --j; // 去掉上一级目录
            i += 2;
        } else {
            buf[j++] = static_cast<char>(c);
        }
    }
    if (j == 0) {
        buf[0] = is_abs ? sep : '.';
        j = 1;
    }
    buf[j] = '\0';
    return Str(reinterpret_cast<uint8_t*>(buf), j);
}

/// 拼接路径，至少需要一个参数
/// 示例: pathcat(mut_path, "dir", "file.txt");
inline auto pathcat(MutStr& path, Str part) noexcept -> void {
    // 追加一个路径片段
    if (part.is_empty()) return;
    char sep =
#if $IsWindows
        '\\';
#else
        '/';
#endif
    // 如果 path 非空且末尾不是分隔符，则添加分隔符
    if (path.size() > 0 && path[path.size()-1] != static_cast<uint8_t>(sep) &&
        path[path.size()-1] != '/')
    {
        path.push_back(static_cast<uint8_t>(sep));
    }
    // 跳过 part 开头的分隔符
    uint32_t start = 0;
    while (start < part.len && (part[start] == '/' || part[start] == '\\')) ++start;
    path.append(part.substr(start, part.len - start));
}

/// 可变参数版路径拼接
template <typename... Args>
auto pathcat(MutStr& path, Str first, Args&&... rest) noexcept -> void {
    pathcat(path, first);
    pathcat(path, static_cast<Str>(rest)...);
}

/// 从完整路径提取文件名
inline auto base_filename(Str path) noexcept -> Str {
    if (path.is_empty()) return path;
    const uint8_t* data = path.ptr.get();
    uint32_t len = path.len;

    // 查找最后一个分隔符
    int32_t last_sep = -1;
    for (uint32_t i = 0; i < len; ++i) {
        if (data[i] == '/' || data[i] == '\\') {
            last_sep = static_cast<int32_t>(i);
        }
    }
    if (last_sep < 0) return path;  // 没有目录部分，整个就是文件名
    uint32_t start = static_cast<uint32_t>(last_sep + 1);
    if (start >= len) return Str();  // 以分隔符结尾，无文件名
    return Str(path.ptr + start, len - start);
}

/// 判断是否为绝对路径
inline auto is_abspath(Str path) noexcept -> bool {
    if (path.is_empty()) return false;
#if $IsWindows
    // Windows: 盘符形式 C: 或 \\ 开头
    if (path.len >= 2 && path[1] == ':') return true;
    if (path[0] == '\\' && path.len >= 2 && path[1] == '\\') return true;
    return false;
#else
    return path[0] == '/';
#endif
}

/// 读取文件内容到可变字符串 out 中，返回错误码
inline auto readfile(Str path, MutStr& out) noexcept -> KstdIOError {
    // 使用 C 文件 API
#if $IsWindows
    // Windows 需要处理宽字符，这里简单使用 fopen (可能仅支持 ASCII 路径)
    FILE* f = ::fopen(path.c_str(), "rb");
#else
    FILE* f = ::fopen(path.c_str(), "rb");
#endif
    if (f == nullptr) {
        // 根据 errno 简单分类
        if (errno == ENOENT) return KstdIOError::FileNotFound;
        if (errno == EACCES || errno == EPERM) return KstdIOError::AccessDenied;
        return KstdIOError::Unknown;
    }

    // 获取文件大小
    ::fseek(f, 0, SEEK_END);
    long fsize = ::ftell(f);
    ::fseek(f, 0, SEEK_SET);

    if (fsize < 0) {
        ::fclose(f);
        return KstdIOError::ReadError;
    }

    out.clear();
    out._buf.reserve(static_cast<uint32_t>(fsize + 1));

    // 逐块读取
    char buf[4096];
    size_t bytes_read;
    while ((bytes_read = ::fread(buf, 1, sizeof(buf), f)) > 0) {
        out.append(Str(reinterpret_cast<uint8_t*>(buf), static_cast<uint32_t>(bytes_read)));
    }

    if (::ferror(f)) {
        ::fclose(f);
        return KstdIOError::ReadError;
    }
    ::fclose(f);
    return KstdIOError::None;
}

/// 将字符串写入文件（覆盖模式）
inline auto writefile(Str path, Str text) noexcept -> KstdIOError {
    FILE* f = ::fopen(path.c_str(), "wb");
    if (f == nullptr) {
        if (errno == EACCES || errno == EPERM) return KstdIOError::AccessDenied;
        return KstdIOError::WriteError;
    }
    if (text.len > 0) {
        size_t written = ::fwrite(text.ptr.get(), 1, text.len, f);
        if (written != text.len) {
            ::fclose(f);
            return KstdIOError::WriteError;
        }
    }
    ::fclose(f);
    return KstdIOError::None;
}

// ===-----------------------
//
// 枚举名称提取（纯编译期实现，仅依赖 C 库）
//
// ===-----------------------

namespace detail {

// 编译期字符串工具
constexpr size_t cstr_len(const char* s) noexcept {
    size_t i = 0;
    while (s[i] != '\0') ++i;
    return i;
}

constexpr const char* cstr_find(const char* s, char c) noexcept {
    while (*s != '\0') {
        if (*s == c) return s;
        ++s;
    }
    return nullptr;
}

constexpr const char* cstr_find_str(const char* haystack, const char* needle) noexcept {
    size_t needle_len = cstr_len(needle);
    if (needle_len == 0) return haystack;
    while (*haystack != '\0') {
        bool found = true;
        for (size_t i = 0; i < needle_len; ++i) {
            if (haystack[i] != needle[i]) {
                found = false;
                break;
            }
        }
        if (found) return haystack;
        ++haystack;
    }
    return nullptr;
}

// 从编译器签名提取单个枚举值名称
template <auto V>
constexpr auto enum_item_name() noexcept -> Str {
    constexpr auto sig = [] {
#if defined(__clang__) || defined(__GNUC__)
        return __PRETTY_FUNCTION__;
#else
        return __FUNCSIG__;
#endif
    }();

    constexpr const char* start = nullptr;
    constexpr const char* end   = nullptr;

    // GCC / Clang 签名示例：
    //   "constexpr auto kstd::detail::enum_item_name() [with auto V = Color::Red]"
    if constexpr (constexpr auto v_eq = cstr_find_str(sig, "V = "); v_eq != nullptr) {
        start = v_eq + 4;                     // 跳过 "V = "
        end   = cstr_find(start, ']');
    }
    // MSVC 签名示例：
    //   "auto __cdecl kstd::detail::enum_item_name<Color::Red>(void)"
    else {
        constexpr size_t len = cstr_len(sig);
        constexpr const char* ptr = sig + len;
        while (ptr != sig) {
            --ptr;
            if (*ptr == '<') {
                start = ptr + 1;
                end   = cstr_find(start, '>');
                break;
            }
        }
        // 跳过可能的 "enum " 前缀
        if constexpr (start != nullptr && end != nullptr && start < end) {
            constexpr const char* enum_kw = "enum ";
            if constexpr (cstr_find_str(start, enum_kw) == start) {
                start += 5;
            }
        }
    }

    if constexpr (start != nullptr && end != nullptr && start < end) {
        // 去除命名空间/类作用域，只保留最后的标识符
        constexpr const char* name_start = start;
        constexpr const char* last_colon = nullptr;
        for (const char* p = start; p < end - 1; ++p) {
            if (p[0] == ':' && p[1] == ':') {
                last_colon = p;
            }
        }
        if constexpr (last_colon != nullptr) {
            name_start = last_colon + 2;      // 跳过 "::"
        }

        constexpr size_t name_len = end - name_start;
        // 合法性检查：标识符以字母或下划线开头
        if constexpr (name_len > 0 && (
            (name_start[0] >= 'a' && name_start[0] <= 'z') ||
            (name_start[0] >= 'A' && name_start[0] <= 'Z') ||
            name_start[0] == '_'))
        {
            return Str(
                Ptr<uint8_t>(const_cast<uint8_t*>(
                    reinterpret_cast<const uint8_t*>(name_start))),
                static_cast<uint32_t>(name_len));
        }
    }
    return Str();   // 解析失败或非法值
}

// 序列生成
template <int... Is>
struct Seq {};

template <int N, int... Is>
struct GenSeqImpl : GenSeqImpl<N - 1, N - 1, Is...> {};

template <int... Is>
struct GenSeqImpl<0, Is...> {
    using type = Seq<Is...>;
};

template <int N>
using GenSeq = typename GenSeqImpl<N>::type;

// ---------- 生成 129 个枚举名称的静态数组 ----------
template <typename T, T V>
constexpr auto get_enum_name() noexcept -> Str {
    return enum_item_name<V>();
}

template <typename T, int... Is>
constexpr const Str (&make_names_array(Seq<Is...>))[sizeof...(Is)] {
    static constexpr Str arr[sizeof...(Is)] = {
        get_enum_name<T, static_cast<T>(Is)>()... };
    return arr;
}

template <typename T>
constexpr const Str (&get_enum_names())[129] {
    return make_names_array<T>(GenSeq<129>{});
}

} // namespace detail

// 
// 将枚举项转为字符串（0~128 连续 enum class）
// 
template <typename T>
inline auto enumeration_name(T enum_item) noexcept -> Str {
    constexpr const Str (&names)[129] = detail::get_enum_names<T>();

    // 直接转为 int 索引
    int idx = static_cast<int>(enum_item);
    if (idx >= 0 && idx <= 128) {
        return names[idx];
    }
    return Str();
}


} // namespace kstd

// ===-----------------------
//
// 清理所有 $ 前缀宏
//
// ===-----------------------
#if $NoMacro
#   undef $ForeachVec
#   undef $Likely
#   undef $Unlikely
#   undef $Assert
#   undef $AssertEq
#   undef $Unimplement
#   undef $Unreachable
#   undef $IsWindows
#   undef $IsLinux
#   undef $IsMac
#endif