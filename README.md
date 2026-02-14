<div align="center">
<image src="https://pub-141831e61e69445289222976a15b6fb3.r2.dev/Image_to_url_V2/HEIF---imagetourl.cloud-1770101835669-relrd5.jpeg" width=160 height=220/>

<h1> kiz v0.7.0 🎉</h1>
</div>

📌 **现状: 修复bug与预发布正式版本(2026.1)...**

## 语言核心定位
kiz-lang 是一门 **面向对象（原型链模型）、强类型+动态类型(鸭子类型)** 的轻量化脚本语言

**使用C++开发**，采用「半编译半解析」架构，内置**栈式虚拟机**（VM）

内存安全，并有基于原子引用计数（atomic reference count）的对象模型。

## 核心设计亮点
- 通过对象的 `__parent__` 属性绑定上级对象，实现原型链继承
- 支持运算符重载与魔术方法
- int 类型为无限精度整数
- 小数类型为Decimal精准小数
- 字符串类型为utf-8字符串且支持f-string
- 🪄 多范式兼容：支持OOP、FP等主流编程范式
- 🔅 语法极简：关键字集高度精简
- ✅ 规范友好：中文注释+统一命名规范
- ✔️ 开发者友好：低门槛快速上手

## 📚 文档完善
- [快速开始](docs/快速开始.md)
- [kiz2026语法与特性文档](http://doc.kiz.random321.com)
- [从源代码构建kiz指南](docs/从源代码构建kiz指南.md)
- [xmake构建教程](docs/xmake构建教程.md)
- [内置对象&库速览](docs/内置对象&库速览.md)
- [使用ai写kiz程序指南](docs/使用ai写kiz程序指南.md)
- [项目结构与功能说明](docs/项目结构与功能说明.md)
- [kiz库开发指南](docs/kiz库开发指南.md)
- [提交issue/bug-report/feature-request/pull-request指南](docs/提交指南.md)

## 🔆 项目结构
- **ArgParser**: 解析控制台参数
- **REPL**: 交互式环境
- **Lexer**: 把源代码解析为token流(基于FSM)
- **Parser**: 把token流解析为抽象语法树(基于朴素递归下降)
- **IRGenerator**: 把抽象语法树解析为字节码
- **VM**: 执行字节码(栈式虚拟机集成GC)
- **Models**: 运行时对象模型系统(包含基于ARC的GC)
- **Builtins**: 内置对象/函数
- **SrcManager**: kiz代码源文件管理器
- **ErrorReporter**: 错误格式化与打印器
- **Depends**: 非业务工具类(Bigint, Decimal, U8String, HashMap, Dict)

## 📃 TODO
- 完善标准库

## 🪄 在线体验
- **官网**: [kiz.random321.com](http://kiz.random321.com)
- **在线文档**: [doc.kiz.random321.com](http://doc.kiz.random321.com)
- **在线运行代码**: [try.kiz.random321.com](http://try.kiz.random321.com)

## 🧷 下载链接
- **下载vscode插件**: [#](#)
- **下载源代码** : [#](#)
- **下载安装包(windows)**: [#](#)
- **下载安装包(linux)**: [#](#)
- **下载安装包(mac os)**: [#](#)

## 📧 联系
kiz-lang@outlook.com