#pragma once
#include <Arduino.h>

class Logger {
public:
    enum OverflowMode : uint8_t {
        DropNewest,
        DropOldest
    };

    // Singleton access
    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    void begin(Stream& out,
        uint16_t flushIntervalMs = 200,
        uint16_t maxBytesPerFlush = 64,
        OverflowMode mode = DropOldest) {
        _out = &out;
        _flushIntervalMs = flushIntervalMs;
        _maxBytesPerFlush = maxBytesPerFlush;
        _overflowMode = mode;
        _lastFlushMs = millis();
    }

    void update() {
        if (!_out) return;

        const uint32_t now = millis();
        if ((uint32_t)(now - _lastFlushMs) < _flushIntervalMs) return;

        flushSome(_maxBytesPerFlush);
        _lastFlushMs = now;
    }

    void flushAll() {
        if (!_out) return;
        while (_count > 0) {
            flushSome(_maxBytesPerFlush);
            yield();
        }
    }

    void setFlushInterval(uint16_t ms) { _flushIntervalMs = ms; }
    void setMaxBytesPerFlush(uint16_t n) { _maxBytesPerFlush = n; }
    void setOverflowMode(OverflowMode m) { _overflowMode = m; }

    void log(const __FlashStringHelper* msg) { writeFlash(msg); writeByte('\n'); }
    void log(const String& msg) { writeStr(msg.c_str()); writeByte('\n'); }
    void log(const char* msg) { writeStr(msg); writeByte('\n'); }

    void printf(const char* fmt, ...) {
        char tmp[96];
        va_list args;
        va_start(args, fmt);
        vsnprintf(tmp, sizeof(tmp), fmt, args);
        va_end(args);
        writeStr(tmp);
    }

    size_t buffered() const { return _count; }
    size_t capacity() const { return BufferSize; }

private:
    Logger()
        : _out(nullptr),
        _flushIntervalMs(200),
        _lastFlushMs(0),
        _overflowMode(DropOldest),
        _head(0), _tail(0), _count(0),
        _maxBytesPerFlush(64) {
    }

    // prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static constexpr size_t BufferSize = 512;
    uint8_t _buf[BufferSize];

    Stream* _out;
    uint16_t _flushIntervalMs;
    uint32_t _lastFlushMs;
    OverflowMode _overflowMode;

    size_t _head;
    size_t _tail;
    size_t _count;
    uint16_t _maxBytesPerFlush;

    void pushByte(uint8_t b) {
        if (_count >= BufferSize) {
            if (_overflowMode == DropNewest) return;

            // DropOldest
            _tail = (_tail + 1) % BufferSize;
            _count--;
        }

        _buf[_head] = b;
        _head = (_head + 1) % BufferSize;
        _count++;
    }

    void writeByte(uint8_t b) { pushByte(b); }
    void writeStr(const char* s) { while (*s) pushByte((uint8_t)*s++); }

    void flushSome(uint16_t maxBytes) {
        if (!_out || _count == 0) return;

        uint16_t toSend = min((uint16_t)_count, maxBytes);

        while (toSend > 0) {
            uint16_t untilWrap = (uint16_t)(BufferSize - _tail);
            uint16_t chunk = min(toSend, untilWrap);

            size_t written = _out->write(&_buf[_tail], chunk);

            // don't block if serial can't accept more
            if (written == 0) break;

            _tail = (_tail + written) % BufferSize;
            _count -= written;
            toSend -= (uint16_t)written;
        }
    }


    void writeFlash(const __FlashStringHelper* msg) {
        PGM_P p = reinterpret_cast<PGM_P>(msg);
        while (true) {
            char c = pgm_read_byte(p++);
            if (!c) break;
            pushByte((uint8_t)c);
        }
    }
};
