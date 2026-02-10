# xmake构建教程

本文将讲述``xmake.lua``的编写思路，以及编译、运行方法。有关``xmake``的更多通用方法可另行搜索资料。

## 编写思路

由于本项目的代码在开发过程中适配了``cmake``，因此``xmake.lua``的编写也得适配``cmake``。其适配动作具体体现在：将``src/version.hpp.in``中的文本替换掉版本号，并将替换后的文本写入到``cmake-build-debug/include/version.hpp``下。

在生成对应版本号以后，就开始了正式的编译：将一系列``.cpp``文件加入到链接当中，并添加一系列文件搜索路径即可。我采用的是``-O2 -static -lm``的编译参数。

## 构建kiz

如果要重新构建``kiz``，请先尝试``xmake clean``来清理缓存。

在正式构建之前，请先执行指令``xmake f ...``来配置工具链和构建模式等细节。

例如：

```bash
xmake f --toolchain=gcc -m release
```

该指令将工具链指定为``gcc``，且编译为发行版。

接下来，执行指令：

```bash
xmake build version
```

来构建版本（即上述的``version.hpp``）。

最后，执行指令：

```bash
xmake build kiz
```

来构建程序，这一步骤可能会耗费较长的时间，特别是初次编译。

当``xmake``输出``[100%]: build ok, ...``的时候，就说明编译成功了，此时会在``build``的一系列子目录下生成一个可执行文件，这就是``kiz``的程序本体。

## 运行kiz

执行指令：

```bash
xmake run kiz
```

即可运行``kiz``的``REPL``程序。也可以尝试提取出可执行文件，这里不再赘述。