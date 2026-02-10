# 从源代码构建kiz指南
本文基于项目 `CMakeLists.txt`，说明如何从源码构建出：
- Windows：`kiz.exe`
- Linux / macOS：`kiz.elf`
- WebAssembly：`kiz_wasm.wasm` + `kiz_wasm.js`

---

## 一、环境要求
- CMake ≥ 3.10
- 支持 C++20 的编译器
  - Windows：MSVC 2019+
  - Linux：GCC 10+ / Clang 11+
  - macOS：Xcode Clang
- 如需构建 WASM：需安装 [Emscripten](https://emscripten.org/)

---

## 二、本地构建（生成 exe / elf）
适用于 Windows、Linux、macOS 本地命令行版本。

### 2.1 构建步骤
1. 创建并进入构建目录
   ```bash
   mkdir build
   cd build
   ```

2. 生成构建文件
   ```bash
   cmake ..
   ```

3. 开始编译
   ```bash
   # Linux / macOS
   make -j$(nproc)

   # Windows（全终端通用）
   cmake --build . --config Release
   ```

### 2.2 输出文件
- Windows：`build/kiz.exe`
- Linux / macOS：`build/kiz.elf`

---

## 三、WebAssembly 构建（wasm）
构建可在浏览器 / Node.js 运行的 WASM 版本。

### 3.1 前置：激活 Emscripten
```bash
# 进入 emsdk 目录
emsdk activate latest

# 生效环境
# Windows
emsdk_env.bat
# Linux/macOS
source ./emsdk_env.sh
```

### 3.2 构建步骤
1. 创建 WASM 构建目录
   ```bash
   mkdir build_wasm
   cd build_wasm
   ```

2. 开启 WASM 构建
   ```bash
   cmake .. -DBUILD_WASM=ON
   ```

3. 编译
   ```bash
   cmake --build .
   ```

### 3.3 输出文件
`build_wasm/` 下会生成：
- `kiz_wasm.wasm`
- `kiz_wasm.js`（ES6 模块化）

---

## 四、CMake 关键说明
- 默认使用 **C++20** 标准
- 自动生成版本头文件：`build/include/version.hpp`
- WASM 模式会自动排除 CLI/REPL 相关文件，只构建核心运行时
- macOS 版本会自动链接 `CoreFoundation`、`CoreGraphics` 框架