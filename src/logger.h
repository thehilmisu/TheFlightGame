#pragma once

#include <cstdio>

#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#define EXPORT_FN __declspec(dllexport)
#elif __linux__
#define DEBUG_BREAK() __builtin_debugtrap()
#define EXPORT_FN
#elif __APPLE__
#define DEBUG_BREAK() __builtin_trap()
#define EXPORT_FN
#endif


enum TextColor {
    TEXT_COLOR_BLACK,
    TEXT_COLOR_RED,
    TEXT_COLOR_GREEN,
    TEXT_COLOR_YELLOW,
    TEXT_COLOR_BLUE,
    TEXT_COLOR_MAGENTA,
    TEXT_COLOR_CYAN,
    TEXT_COLOR_WHITE,
    TEXT_COLOR_BRIGHT_BLACK,
    TEXT_COLOR_BRIGHT_RED,
    TEXT_COLOR_BRIGHT_GREEN,
    TEXT_COLOR_BRIGHT_YELLOW,
    TEXT_COLOR_BRIGHT_BLUE,
    TEXT_COLOR_BRIGHT_MAGENTA,
    TEXT_COLOR_BRIGHT_CYAN,
    TEXT_COLOR_BRIGHT_WHITE,
    TEXT_COLOR_COUNT
};

template<typename... Args>
void _log(const char *prefix, const char *msg, const TextColor textColor, Args... args) {
    static constexpr const char *TextColorTable[TEXT_COLOR_COUNT] = {
        "\x1b[30m", "\x1b[31m", "\x1b[32m", "\x1b[33m",
        "\x1b[34m", "\x1b[35m", "\x1b[36m", "\x1b[37m",
        "\x1b[90m", "\x1b[91m", "\x1b[92m", "\x1b[93m",
        "\x1b[94m", "\x1b[95m", "\x1b[96m", "\x1b[97m",
    };

    char msgBuffer[8192];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
    snprintf(msgBuffer, sizeof(msgBuffer), msg, args...);
#pragma clang diagnostic pop

    char textBuffer[8192];
    snprintf(textBuffer, sizeof(textBuffer), "%s %s %s \033[0m",
             TextColorTable[textColor], prefix, msgBuffer);

    puts(textBuffer);
}

#define INFO(msg, ...)  _log("INFO:",  (const char*)msg, TEXT_COLOR_BRIGHT_GREEN, ##__VA_ARGS__)
#define TRACE(msg, ...) _log("TRACE:", (const char*)msg, TEXT_COLOR_GREEN,        ##__VA_ARGS__)
#define WARN(msg, ...)  _log("WARN:",  (const char*)msg, TEXT_COLOR_YELLOW,       ##__VA_ARGS__)
#define ERROR(msg, ...) _log("ERROR:", (const char*)msg, TEXT_COLOR_RED,          ##__VA_ARGS__)

#define ASSERT(x, msg, ...)             \
do {                                    \
    if(!(x)) {                          \
        ERROR(msg, ##__VA_ARGS__);      \
        DEBUG_BREAK();                  \
        ERROR("Assertion HIT!");        \
    }                                   \
} while(0)
