#pragma once
#include <iostream>
#include <string>
#include "repl/color.hpp"
#include "version.hpp"

// 调试模式开关
#define IN_DEBUG

//#undef IN_DEBUG

#ifdef IN_DEBUG
#define DEBUG_OUTPUT(msg) \
    do { \
        std::cout << Color::BRIGHT_YELLOW \
        << "[DEBUG] " << __FILE__ << ":" << __LINE__ << " | " \
        << "msg: " << (msg) << Color::RESET << std::endl; \
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