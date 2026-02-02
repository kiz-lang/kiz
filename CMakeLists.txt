cmake_minimum_required(VERSION 3.10)

project(kiz
        VERSION 0.5.0
        LANGUAGES CXX)

# ===================== 基础配置 =====================
# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 版本号定义
set(KIZ_VERSION_MAJOR 0)
set(KIZ_VERSION_MINOR 5)
set(KIZ_VERSION_PATCH 0)
set(KIZ_VERSION "${KIZ_VERSION_MAJOR}.${KIZ_VERSION_MINOR}.${KIZ_VERSION_PATCH}")

# 配置版本头文件（生成到构建目录）
# 注意：如果 version.hpp.in 实际在 include/ 下，改为 ${PROJECT_SOURCE_DIR}/include/version.hpp.in
configure_file(
        ${PROJECT_SOURCE_DIR}/src/version.hpp.in  # 按你实际路径调整
        ${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp
        @ONLY
)

# ===================== 手动枚举所有 .cpp 文件（仅列cpp，无hpp） =====================
set(SRC_FILES
        # 入口文件
        ${PROJECT_SOURCE_DIR}/src/main.cpp

        # 编译器模块
        ${PROJECT_SOURCE_DIR}/src/lexer/lexer.cpp
        ${PROJECT_SOURCE_DIR}/src/parser/parser.cpp
        ${PROJECT_SOURCE_DIR}/src/parser/parse_expr.cpp
        ${PROJECT_SOURCE_DIR}/src/parser/parse_stmt.cpp

        # IR 生成模块
        ${PROJECT_SOURCE_DIR}/src/ir_gen/ir_gen.cpp
        ${PROJECT_SOURCE_DIR}/src/ir_gen/gen_expr.cpp
        ${PROJECT_SOURCE_DIR}/src/ir_gen/gen_stmt.cpp

        # VM 核心模块
        ${PROJECT_SOURCE_DIR}/src/vm/vm.cpp
        ${PROJECT_SOURCE_DIR}/src/vm/exec_get_set.cpp
        ${PROJECT_SOURCE_DIR}/src/vm/exec_calc.cpp
        ${PROJECT_SOURCE_DIR}/src/vm/exec_call.cpp
        ${PROJECT_SOURCE_DIR}/src/vm/exec_misc.cpp
        ${PROJECT_SOURCE_DIR}/src/vm/entry_std_modules.cpp
        ${PROJECT_SOURCE_DIR}/src/vm/entry_builtins.cpp
        ${PROJECT_SOURCE_DIR}/src/vm/handle_error.cpp
        ${PROJECT_SOURCE_DIR}/src/vm/execute_unit.cpp

        # 工具模块
        ${PROJECT_SOURCE_DIR}/src/error/error_reporter.cpp
        ${PROJECT_SOURCE_DIR}/src/util/src_manager.cpp
        ${PROJECT_SOURCE_DIR}/src/repl/repl.cpp
        ${PROJECT_SOURCE_DIR}/src/repl/repl_readline.cpp
)

# lib 模块（仅列cpp）
set(LIB_SRC_FILES
        ${PROJECT_SOURCE_DIR}/libs/builtins/bool_methods.cpp
        ${PROJECT_SOURCE_DIR}/libs/builtins/int_methods.cpp
        ${PROJECT_SOURCE_DIR}/libs/builtins/decimal_methods.cpp
        ${PROJECT_SOURCE_DIR}/libs/builtins/nil_methods.cpp
        ${PROJECT_SOURCE_DIR}/libs/builtins/str_methods.cpp
        ${PROJECT_SOURCE_DIR}/libs/builtins/list_methods.cpp
        ${PROJECT_SOURCE_DIR}/libs/builtins/dict_methods.cpp
        ${PROJECT_SOURCE_DIR}/libs/builtins/builtin_functions.cpp
        ${PROJECT_SOURCE_DIR}/libs/io/io_lib.cpp


)

# ===================== 文件存在性检查（仅检查cpp） =====================
foreach(src_file ${SRC_FILES})
    if(NOT EXISTS ${src_file})
        message(FATAL_ERROR "[错误] 未找到src目录文件：${src_file}，请检查路径/文件名是否正确！")
    endif()
endforeach()

foreach(lib_file ${LIB_SRC_FILES})
    if(NOT EXISTS ${lib_file})
        message(FATAL_ERROR "[错误] 未找到库文件：${lib_file}，请检查路径/文件名是否正确！")
    endif()
endforeach()

# 合并所有cpp文件
set(ALL_SRC_FILES ${SRC_FILES} ${LIB_SRC_FILES}
        src/vm/exec_import.cpp)

# ===================== 编译配置（核心：让编译器找到hpp） =====================
# 仅添加cpp到可执行目标
add_executable(kiz ${ALL_SRC_FILES})

# 关键：指定头文件搜索路径 → 编译器会在这些路径下自动找#include的hpp
target_include_directories(kiz
        PRIVATE
        # src根目录
        ${PROJECT_SOURCE_DIR}/src

        # 依赖库目录
        ${PROJECT_SOURCE_DIR}/deps

        ${PROJECT_SOURCE_DIR}/libs

        # 生成的version.hpp所在目录
        ${CMAKE_CURRENT_BINARY_DIR}/include
)

# 平台相关后缀
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set_target_properties(kiz PROPERTIES SUFFIX ".exe")
else()
    set_target_properties(kiz PROPERTIES SUFFIX ".elf")
endif()

# Xlib
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_package(X11 REQUIRED)
    target_include_directories(kiz PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(kiz ${X11_LIBRARIES})
endif()

# ===================== 编译信息打印 =====================
list(LENGTH ALL_SRC_FILES TOTAL_SRC_COUNT)
list(LENGTH SRC_FILES SRC_DIR_COUNT)
list(LENGTH LIB_SRC_FILES LIB_FILES_COUNT)

message(STATUS "======================================")
message(STATUS "  项目 kiz v${PROJECT_VERSION} 编译配置")
message(STATUS "======================================")
message(STATUS "✅ C++标准：C++20")
message(STATUS "✅ 仅添加 .cpp 文件到编译目标，.hpp 自动查找")
message(STATUS "✅ 源文件总数：${TOTAL_SRC_COUNT}")
message(STATUS "  - src目录：${SRC_DIR_COUNT} 个 .cpp 文件")
message(STATUS "  - libs：${LIB_FILES_COUNT} 个 .cpp 文件")
message(STATUS "✅ 目标文件路径：${CMAKE_BINARY_DIR}/kiz$<TARGET_FILE_SUFFIX:kiz>")
message(STATUS "✅ 编译器自动搜索 .hpp 的路径：")
message(STATUS "  - ${PROJECT_SOURCE_DIR}/src")
message(STATUS "  - ${PROJECT_SOURCE_DIR}/deps（依赖库hpp）")
message(STATUS "  - ${PROJECT_SOURCE_DIR}/libs")
message(STATUS "  - ${CMAKE_CURRENT_BINARY_DIR}/include（生成的version.hpp）")
message(STATUS "======================================")
