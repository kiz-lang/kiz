# Kiz-lang v0.2.1
📌 **现状: 开发中...**

- 📚 文档完善
- 🪄 多范式兼容：支持OOP、FP等主流编程范式
- 🔅 语法极简：关键字集高度精简，仅包含：
```kiz
if else while break next
fn end import
try catch throw 
nonlocal global
is not or and in
True Nil False
```
- ✅ 规范友好：中文注释+统一命名规范
- ✔️ 开发者友好：低门槛快速上手
- 🔆 结构
    
    - **ArgParser**: 解析控制台参数
    - **REPL**: 交互式环境
    - **Lexer**: 把源代码解析为token流
    - **Parser**: 把token流解析为抽象语法树
    - **IRGenerator**: 把抽象语法树解析为字节码
    - **VM**: 执行字节码
    - **Models**: 运行时对象模型系统
    - **Builtins**: 内置对象/函数

- 🔹 功能

- 📃 TODO: 

    **已完成的**
    - ~~**fixme** user function的调用问题~~ (感谢三文鱼)
    - ~~**fixme** 修复Nil, False, True作为字面量出现的undefined var问题~~
    - ~~**feature** 完成list的IR生成~~
    - ~~**feature** 实现getattr~~
    - ~~**feature** 实现setattr~~
    - ~~**feature** 实现call method~~
    - ~~**feature** 完成 and not or in运算符(在vm中要支持判断model::Bool, 如果对象不是model::Bool, 需尝试调用Object.__bool__魔术方法)~~
    - ~~**fixme[急需的]** if, while 语句的跳转问题~~
    - ~~**feature** 实现next, break~~
    -- ~~**feature[急需的]** 实现oop支持~~
    - ~~**feature[急需的]** 添加支持TraceBack的报错器~~
    - ~~**feature[急需的]** 通过kiz::Position(已经在kiz.hpp定义了这个结构体)这个结构体来储存token, ast, instruction的位置信息~~
    - ~~**test[急需的]** 测试nonlocal和global语句, lambda定义和and/or/not~~
    - ~~- **feature** 完成注释功能~~
    ~~- **feature** 完成for语句~~
    ~~- **feature** 完成try-catch throw语句~~

    **之后的计划**
    - **fixme** 确保引用计数正确 
    - **feature** 完成 >= <= != (通过添加操作指令OP_GE, OP_LE, OP_NE)
    - **feature** 完成所有builtin函数
    - **feature** 实现完整oop语法(语句用法见examples/oop.kiz)
    - **feature(maybe has big change)** 所有报错使用util::err_reporter函数代替现在临时的assert
    - **fixme(maybe has big change)** 统一报错和DEBUG信息和输出信息为标准英文
    - **feature(maybe has big change)** Object->to_string改为各对象模型的魔术方法(`__str__`和`__dstr__`)并为list添加`copy`方法
    
    - **feature** 添加import(语句形式:`import "path"`与`import mod_name`并存)及其相关的`IMPORT <name_idx>`字节码指令(注意vm.hpp已有相关预留), 循环导入检查, 形如`mod.func()`的模块属性调用系统(注意：通过在找不到变量时通过`CallFrame.ower, getattr(ower, "__ower_module__")`(通过查找对象的`__ower_module__`属性获取所属模块), 以实现模块函数内部不带模块名访问模块内部成员功能), std模块系统(在model::std_modules中注册)和用户模块系统
    - **feature** 完善builtins object的, `__getitem__`, `__setitem__`, `__str__`, `__dstr__`这些魔术方法, 同时支持用户定义的魔术方法
    - **faeture** 完成管道运算符
