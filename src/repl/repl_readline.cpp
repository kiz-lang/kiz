#include <iosfwd>
#include <string>
#include <chrono>
#include <istream>
#include "kiz.hpp"
#include "repl.hpp"
#include "color.hpp"

// 跨平台头文件
#ifdef _WIN32
    #include <windows.h>
#elif __APPLE__
    #include <ApplicationServices/ApplicationServices.h>
#elif __linux__
// Linux 纯sys API 依赖，无X11
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <sys/stat.h>
    #include <linux/input.h>
    #include <dirent.h>
    #include <cstring>
    #include <cstdint>
#endif

#include <stdbool.h>


bool ui::if_pressing_shift() {

#ifdef _WIN32
    // Windows 原逻辑保留
    return ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0);

#elif __APPLE__
    // macOS 原逻辑保留
    CGEventRef event = CGEventCreate(NULL);
    CGEventFlags flags = CGEventGetFlags(event);
    CFRelease(event);
    return (flags & kCGEventFlagMaskShift) != 0;

#elif __linux__
    // Linux 纯sys API实现：输入子系统 + ioctl获取按键状态
    static int kbd_fd = -1; // 静态保存键盘设备fd，避免重复初始化
    constexpr size_t KEY_BUF_SIZE = KEY_MAX / 8 + 1;

    // 辅助函数：遍历/dev/input，找到第一个键盘设备（内部使用）
    static int find_keyboard_device() {
        DIR* dir = opendir("/dev/input");
        if (!dir) return -1;

        struct dirent* entry = nullptr;
        while ((entry = readdir(dir)) != nullptr) {
            // 只处理event*设备文件
            if (strncmp(entry->d_name, "event", 5) != 0) continue;

            // 拼接设备路径
            char dev_path[64] = {0};
            snprintf(dev_path, sizeof(dev_path), "/dev/input/%s", entry->d_name);

            // 以只读+非阻塞模式打开
            int fd = open(dev_path, O_RDONLY | O_NONBLOCK);
            if (fd < 0) continue;

            // 获取设备名称，判断是否为键盘（包含Keyboard/kbd/keyboard）
            char dev_name[256] = {0};
            if (ioctl(fd, EVIOCGNAME(sizeof(dev_name) - 1), dev_name) >= 0) {
                if (strstr(dev_name, "Keyboard") || strstr(dev_name, "keyboard") || strstr(dev_name, "kbd")) {
                    closedir(dir);
                    return fd; // 找到键盘设备，返回fd
                }
            }

            close(fd); // 非键盘设备，关闭fd
        }

        closedir(dir);
        return -1; // 未找到键盘设备
    }

    // 初始化：首次调用时找到并打开键盘设备
    if (kbd_fd < 0) {
        kbd_fd = find_keyboard_device();
        if (kbd_fd < 0) return false;
    }

    // 获取当前所有按键的状态（核心ioctl：EVIOCGKEY）
    uint8_t key_state[KEY_BUF_SIZE] = {0};
    if (ioctl(kbd_fd, EVIOCGKEY, key_state) < 0) {
        close(kbd_fd); // ioctl失败，重置fd
        kbd_fd = -1;
        return false;
    }

    // 检测左Shift(KEY_LEFTSHIFT=42)、右Shift(KEY_RIGHTSHIFT=54)是否按下
    // 按键状态：位为1表示按下，0表示松开
    bool lshift = (key_state[KEY_LEFTSHIFT / 8] & (1 << (KEY_LEFTSHIFT % 8))) != 0;
    bool rshift = (key_state[KEY_RIGHTSHIFT / 8] & (1 << (KEY_RIGHTSHIFT % 8))) != 0;

    return lshift || rshift;

#else
    // 其他未知平台
    return false;
#endif
}

/**
 * @brief 使REPL接收完整输入
 * @param is 输入流
 * @param os 输出流
 * @result 完整输入
 */
std::string ui::get_whole_input(std::istream *is, std::ostream *os) {
    if (!is) {
        throw KizStopRunningSignal("istream pointer is null");
    }

    std::string input;
    char ch;

    while (is->get(ch)) { // 直接判断流状态，get失败（eof/fail）时退出
        // 使用Shift+Enter组合键继续输入
        if (if_pressing_shift() && ch == '\n') {
            *os << Color::BRIGHT_MAGENTA << "... " << Color::RESET;
            os->flush();
            input += ch;
            DEBUG_OUTPUT("Add \\n to input: " << input);
        } else if (ch == '\n') { // Enter结束输入
            DEBUG_OUTPUT("final returns input: " << input);
            return input;
        } else if (static_cast<unsigned char>(ch) != 0xFF) { // 过滤无效字符
            input += ch;
        }
    }

    // 处理EOF（用户按Ctrl+D/Ctrl+Z）
    if (input.empty() && is->eof()) {
        throw KizStopRunningSignal("EOF received, exit REPL");
    }

    DEBUG_OUTPUT("final returns input (EOF): " << input);
    return input;
}
