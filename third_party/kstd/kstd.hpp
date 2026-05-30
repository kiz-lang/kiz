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


//
// 非法索引
//
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

//
// 动态数组类型
//
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
        $Unimplement();
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

    /// 弹出末尾
    constexpr auto pop() noexcept -> void {
        if (len > 0) {
            len--;
        }
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


} // namespace kstd

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