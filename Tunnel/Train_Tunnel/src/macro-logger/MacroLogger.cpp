// MacroLogger.cpp
#include "MacroLogger.h"

Logger::Logger() : _level(INFO) {}

void Logger::set_level(Level level) {
    get()._level = level;
}

Logger::Level Logger::get_level() {
    return get()._level;
}

Logger& Logger::get() {
    static Logger logger;
    return logger;
}

void Logger::trace(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log(TRACE, fmt, args);
    va_end(args);
}

void Logger::info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log(INFO, fmt, args);
    va_end(args);
}

void Logger::warning(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log(WARNING, fmt, args);
    va_end(args);
}

void Logger::error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log(ERROR, fmt, args);
    va_end(args);
}

void Logger::critical(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log(CRITICAL, fmt, args);
    va_end(args);
}

void Logger::log(Level level, const char* fmt, va_list argp) {
    // Filter by active log level
    if (get()._level <= level) {
        get().out(level, fmt, argp);
    }
}

void Logger::out(Level level, const char* fmt, va_list argp) {
    char buffer[LOGGER_BUF_SIZE];
    vsnprintf(buffer, sizeof(buffer), fmt, argp);

    const __FlashStringHelper* tag = F("INFO");
    switch (level) {
    case TRACE:    tag = F("TRACE"); break;
    case INFO:     tag = F("INFO"); break;
    case WARNING:  tag = F("WARN"); break;     // shorter tag saves output time
    case ERROR:    tag = F("ERROR"); break;
    case CRITICAL: tag = F("CRIT"); break;     // shorter tag
    }

    Serial.print(F("["));
    Serial.print(tag);
    Serial.print(F("] "));
    Serial.println(buffer);
}
