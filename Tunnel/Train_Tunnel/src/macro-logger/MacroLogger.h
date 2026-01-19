// MacroLogger.h
#pragma once

#include <Arduino.h>
#include <stdarg.h>

// Configurable log line buffer size (stack).
// For ATmega328P (2KB SRAM), 80-128 is a sensible range.
#ifndef LOGGER_BUF_SIZE
#define LOGGER_BUF_SIZE 96
#endif

class Logger {
public:
    enum Level : uint8_t {
        TRACE = 0,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    static Level get_level();
    static void set_level(Level level);

    static void trace(const char* fmt, ...);
    static void info(const char* fmt, ...);
    static void warning(const char* fmt, ...);
    static void error(const char* fmt, ...);
    static void critical(const char* fmt, ...);

    static void log(Level level, const char* fmt, va_list argp);

    static Logger& get();  // reference to singleton instance

private:
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void out(Level level, const char* fmt, va_list argp);

    volatile Level _level;
};

// -----------------------------------------------------------------------------
// User-friendly macros (prefixed to avoid collisions with other libraries).
// Works with or without extra arguments.
// -----------------------------------------------------------------------------

///*
#define LOG_CRITICAL(fmt, ...) Logger::critical("%s:%d " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)    Logger::error   ("%s:%d " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...)  Logger::warning ("%s:%d " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)     Logger::info    ("%s:%d " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_TRACE(fmt, ...)    Logger::trace   ("%s:%d " fmt, __func__, __LINE__, ##__VA_ARGS__)
//*/

/*
#define LOG_CRITICAL(fmt, ...) {}
#define LOG_ERROR(fmt, ...)    {}
#define LOG_WARNING(fmt, ...)  {}
#define LOG_INFO(fmt, ...)     {}
#define LOG_TRACE(fmt, ...)    {}
//*/