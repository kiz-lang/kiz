#pragma once
#include <iostream>
#include <string>
#include "repl/color.hpp"
#include "../cmake-build-debug/include/version.hpp"

// 调试模式开关
#define IN_DEBUG

#undef IN_DEBUG

#ifdef IN_DEBUG
#define DEBUG_OUTPUT(msg) \
    do { \
        std::cout << Color::BRIGHT_YELLOW \
        << "[DEBUG] " << __FILE__ << ":" << __LINE__ << " | " \
        << "msg: " << msg << Color::RESET << std::endl; \
    } while(0)

#else
#define DEBUG_OUTPUT(msg) ((void)0)
#endif

#include <exception>
#include <string>

class KizStopRunningSignal final : public std::runtime_error {
public:
    KizStopRunningSignal() noexcept
        : std::runtime_error("kiz-lang 执行终止信号") {}

    explicit KizStopRunningSignal(const std::string& msg) noexcept
        : std::runtime_error(msg) {}
};

class NativeFuncError final : public std::runtime_error {
public:
    std::string name;
    std::string msg;
    explicit NativeFuncError(std::string  name_, std::string  msg_) noexcept
        : name(std::move(name_)), msg(msg_), std::runtime_error(msg_) {}
};