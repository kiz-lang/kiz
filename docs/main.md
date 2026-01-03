# kiz-lang 官方文档

## 语言核心定位
kiz-lang 是一门 **面向对象（原型链模型）、强类型+动态类型** 的轻量化脚本语言，**使用C++开发**，采用「半编译半解析」架构，内置**栈式虚拟机**（VM）与基于引用计数（reference count）的对象模型。

**核心设计亮点**：
- 通过对象的 `__parent__` 属性绑定上级对象，实现原型链继承
- 支持运算符重载与魔术方法
- int 类型为无限精度整数。

## 目录

[已支持功能清单](##已支持功能清单)

[快速使用指令](##快速使用指令)

[代码示例](##代码示例)

[附录](##附录)

[编译指南(如何把kiz源代码编译为可执行文件)](##编译指南(如何把kiz源代码编译为可执行文件))

[待实现功能规划（下一个版本）](##待实现功能规划（下一个版本）)

[已知局限性（将在下一个版本优化）](##已知局限性（将在下一个版本优化）)

## 已支持功能清单
### 1. 核心语法与结构
- 面向对象编程（OOP）支持（原型链模型）
- 变量定义与赋值（无需关键字，直接声明赋值）
- 函数定义（函数本质为对象）与匿名函数
- 方法定义（首个形参建议命名为 `this`，类似 Python 方法绑定）
- 条件语句：`if-else if-else`
- 循环语句：`while`
- 流程控制：`next`（等价于 continue）、`break`、`return`
- 跨作用域变量控制：`nonlocal`/`global` 关键字（语法要求：关键字后必须紧跟赋值语句，如 `nonlocal x = 0` `global m = 100`，与 Python 语法存在差异）
- 单行注释：以 `#` 开头
- 递归支持
- 异常处理：`try-catch` 语句与 `throw` 语句
- 循环语法：`for` 循环（通过调用对象 `__next__` 方法，直至返回 `StopIterException` 对象）
- REPL 交互式运行环境

### 2. 数据类型与字面量
- 基础类型：`int`（无限精度整数）、`rational`（临时有理数类型）、`str`（字符串）、`list`（列表）、`bool`（布尔值：`True`/`False`）、`nil`（空值）
- 类型特性：`int`/`str`/`bool` 重载 `__call__` 方法，支持类型转换（如 `int("1")` `str(1)`）
- 字面量支持：列表、整数、布尔值、`Nil`

### 3. 表达式与运算符
- 算术运算：加减乘除（`+` `-` `*` `/`）、乘方（`^`）
- 比较运算：等于（`==`）、大于（`>`）、小于（`<`）（暂不支持 `>=` `<=` `!=`）
- 逻辑运算：`and` `not` `or`
- 运算符重载：通过魔术方法实现
- 属性访问与设置：`obj.attr` 与 `obj.attr = value`
- 函数/方法调用：`func()` `obj.method()`

### 4. 核心能力与工具
- 魔术方法支持（见附录）
- 错误处理：完整报错信息与 TraceBack 追踪
- 调试能力：`breakpoint()` 函数打断点
- 内置函数（见附录）
- 引用计数查询：`get_refc(obj)` 函数

## 快速使用指令
通过 `kiz.exe` 可执行文件操作，支持以下命令：
```bash
.\kiz.exe          # 启动 REPL 交互式环境
.\kiz.exe repl     # 同上，显式指定 REPL 模式
.\kiz.exe <path>   # 运行指定路径的 .kiz 脚本文件
.\kiz.exe run <path> # 同上，显式指定运行模式
.\kiz.exe version  # 查看当前版本号
.\kiz.exe help     # 查看帮助文档
```

## 代码示例
### 1. 面向对象（OOP）实现
```kiz
# 创建 Point 原型对象
Point = create()

# 重载 __call__ 方法（类似构造函数）
Point.__call__ = fn (this, x, y)
    o = create(this)  # 基于 Point 原型创建实例
    o.x = x
    o.y = y
    return o
end

# 重载 __add__ 方法（实现点的加法）
Point.__add__ = fn (this, other)
    return Point(this.x + other.x, this.y + other.y)
end

# 实例化与运算
a = Point(3, 5)
b = Point(4, 0)
c = a + b
print("Point(x=" + str(c.x) + ", y=" + str(c.y) + ")")  # 输出：Point(x=7, y=5)
```

### 2. 函数调用与跨作用域变量
```kiz
n = 0
fn a
    m = 0
    fn b
        fn c
            nonlocal m = 100  # 修改外层非全局变量
            global n = 99     # 修改全局变量
        end
        c()
    end
    b()
    print(m)  # 输出：100
end
a()
print(n)      # 输出：99
```

### 3. 匿名函数
```kiz
# 匿名函数定义（|参数| 语法）
a = |m| m + 1
print(a(1))  # 输出：2

# 普通函数作为参数
b = fn (param)
    print(param)
end
b(3)  # 输出：3
```

### 4. 条件语句（if-else if-else）
```kiz
a = int(input(">>>"))  # 读取输入并转换为 int 类型
if a == 0
    print("a is 0")
else if a == 2
    print("a is 2")
else
    print("other num")
end
```

### 5. 循环语句（while）与流程控制
```kiz
# next 示例（跳过 i=7）
i = 0
while i < 10
    i = i + 1
    if i == 7
        next
    end
    print(i)  # 输出：1 2 3 4 5 6 8 9 10
end

# break 示例（i=7 时终止循环）
i = 0
while i < 10
    i = i + 1
    if i == 7
        break
    end
    print(i)  # 输出：1 2 3 4 5 6
end
```


## 附录
### 1. 内置函数与对象
| 名称                 | 功能描述                                              |
| ------------------ | ------------------------------------------------- |
| `print(...)`       | 输出参数内容                                            |
| `input(prompt)`    | 接收用户输入，可选提示文本                                     |
| `isinstance(a, b)` | 判断 a 是否是 b 原型的实例（基于 `__parent__` 原型链）             |
| `create(src=Nil)`  | 创建空对象，`__parent__` 属性指向 `src`（未指定则无 `__parent__`） |
| `now()`            | 获取当前时间                                            |
| `get_refc(obj)`    | 获取对象的引用计数                                         |
| `breakpoint()`     | 触发断点调试                                            |
| `int(obj=0)`       | 转换为 int 类型（默认值 0）                                 |
| `bool(obj)`        | 转换为 bool 类型（基于对象 `__bool__` 方法）                   |
| `str(obj="")`      | 转换为 str 类型（默认值空字符串）                               |
| `list()`           | 创建空列表                                             |
| `function`         | 函数类型对象                                            |
| `nil`              | 空值对象                                              |
| `rational`         | 临时有理数类型                                           |

### 2. 支持的表达式与运算符
#### 算术表达式
`a + b` `a - b` `a * b` `a / b` `a ^ b`（乘方）

#### 比较表达式
`a == b` `a > b` `a < b`（暂不支持 `>=` `<=` `!=`）

#### 逻辑表达式
`not a` `a and b` `a or b`

#### 访问与调用表达式
`a.b`（属性访问）`a()`（函数/对象调用）`a.b()`（方法调用）

### 3. 魔术方法清单
| 魔术方法名              | 功能描述                               |
| ------------------ | ---------------------------------- |
| `__parent__`       | 原型链核心属性，指向父对象                      |
| `__add__`          | 重载 `+` 运算符                         |
| `__sub__`          | 重载 `-` 运算符                         |
| `__mul__`          | 重载 `*` 运算符                         |
| `__div__`          | 重载 `/` 运算符                         |
| `__pow__`          | 重载 `^` 运算符（乘方）                     |
| `__mod__`          | 预留：重载取模运算符（暂未实现）                   |
| `__eq__`           | 重载 `==` 运算符                        |
| `__gt__`           | 重载 `>` 运算符                         |
| `__lt__`           | 重载 `<` 运算符                         |
| `__owner_module__` | 标注对象所属模块（用于模块系统查找）                 |
| `__call__`         | 支持对象直接调用（`obj()`）                  |
| `__bool__`         | 支持布尔判断（`if obj`）                   |
| `__str__`          | 转换为字符串（`str(obj)`）                 |
| `__dstr__`         | 返回调试字符串（类似 python 的 `repr`，暂未完全实现） |
| `__getitem__`      | 重载下标访问（`obj[idx]`，暂未实现）            |
| `__setitem__`      | 重载下标赋值（`obj[idx] = val`，暂未实现）      |
| `__next__`         | 迭代器方法（支持 `for` 循环）            |
| `__is_immut__`     | 判断对象可变性（暂未实现，下一个版本 规划）            |

## 编译指南(如何把kiz源代码编译为可执行文件)
### 一、编译前必备条件（需新增准备）
1. 工具依赖：安装 CMake 3.10 及以上版本 + 支持 C++20 标准的编译器（GCC 10+/Clang 11+/MSVC 2019+）
2. 文件校验：确认项目目录结构完整，需包含：
   - `src/main.cpp`（程序入口文件）
   - `libs/builtins/` 目录下所有指定源文件（bool_obj.cpp、int_obj.cpp 等 8 个文件，见 CMakeLists 清单）
   - `include/version.hpp.in` 模板文件（用于生成版本头文件）
3. 目录检查：确保 `deps/` `libs/` 目录存在（即使为空，需符合头文件搜索路径配置）

### 二、新增编译步骤（标准流程）
1. 创建构建目录（推荐，避免污染源码）：
   ```bash
   mkdir build && cd build
   ```
2. 生成编译配置（根据平台自动适配）：
   ```bash
   cmake ..  # Windows 下若需指定编译器，可添加 -G "Visual Studio 16 2019" 等参数
   ```
3. 执行编译（默认生成 Release 版本，需 Debug 可添加 `-DCMAKE_BUILD_TYPE=Debug`）：
   ```bash
   # Windows（Visual Studio）
   cmake --build . --config Release
   # Linux/macOS
   make -j$(nproc)  # -j 后接核心数，加速编译
   ```
4. 产物位置：编译成功后，可执行文件在 `build/` 根目录下（Windows 为 `kiz.exe`，Linux/macOS 为 `kiz.elf`）

### 三、需新增的注意事项
1. 文件名修正：确认 `libs/builtins/` 下的函数文件名为 `builtin_functions.cpp`（CMake 注释提示可能少打一个 `l`，需与实际文件名一致）
2. 路径规范：所有源文件路径需与 CMakeLists 中配置一致，不可随意修改目录结构（否则会触发文件不存在的 FATAL_ERROR）
3. 版本头文件：`version.hpp` 由 CMake 自动生成到 `build/include/`，无需手动创建，确保 `version.hpp.in` 中变量命名与 CMake 一致（如 `@KIZ_VERSION@`）
4. 平台差异：Windows 下需确保编译器支持 C++20 标准（MSVC 需升级到 2019 及以上），Linux/macOS 需手动安装 GCC 10+/Clang 11+

---

## 待实现功能规划（下一个版本）
### 1. 模块系统
- 导入语法：支持 `import "path"` 路径导入与 `import mod_name` 模块名导入
- 字节码支持：新增 `IMPORT <name_idx>` 指令（VM 已预留接口）
- 模块特性：循环导入检查、`mod.func()` 模块属性调用（通过 `CallFrame.owner` 与 `__owner_module__` 属性查找模块）
- 模块体系：`model::std_modules` 标准模块注册 + 用户自定义模块

### 2. 语法扩展
- 下标操作：`list[idx]` 下标访问与赋值语法
- 代码块语法：`object` 语句（创建一个原型并批量设置对象属性，用于类/对象定义简化）
- 比较运算符：补充 `>=` `<=` `!=` 支持

### 3. 内置类型增强
#### 列表（list）方法
`foreach` `reverse` `extend` `pop` `insert` `find` `map` `count` `filter` `__next__`

#### 字符串（str）方法
`startswith` `endswith` `isnum` `isalpha` `find` `map` `count` `filter` `__next__`

### 4. 内置函数完善
`range` `help` `cmd` `typeof` `copy` `setattr` `getattr` `delattr`

### 5. 核心能力补充
- 对象特性：`__is_immut__` 魔术方法（判断对象可变性，决定默认传递方式：不可变对象引用传递，可变对象拷贝传递）
- 基础类型魔术方法完善：`__getitem__` `__setitem__` `__str__` `__dstr__`（`__dstr__` 类似 Python 的 `repr`，返回调试字符串）

## 已知局限性（将在下一个版本优化）
- 部分场景报错依赖 `assert`，未完全实现结构化异常
- REPL 不支持多行输入
- 内置函数（builtins）数量较少
- `rational` 类型为临时方案，后续计划替换为 `decimal` 类型
- 暂不支持 `>=` `<=` `!=` 比较运算符
- 部分报错信息与调试信息存在英文语法错误