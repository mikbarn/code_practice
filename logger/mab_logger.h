#pragma once

#include <iostream>
#include <cstdarg>
#include <memory>
#include <stdexcept>

namespace mab {
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL
    };

    static const char* LevelNames[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};

    class LogEmitter {
        public:
            virtual void emit(LogLevel lvl, const char *fmt, std::va_list args) = 0;
    };

    class ConsoleEmitter: public LogEmitter{
        void emit(LogLevel lvl, const char * fmt, std::va_list args) override;
    };

    class Logger {
        const static size_t MAX_EMITTERS = 8;
        static LogLevel lvl;
        public:
            Logger();
            Logger(LogLevel lvl);
            static void setLevel(LogLevel level);
            void addEmitter(std::unique_ptr<LogEmitter> ptr);

            void trace(const char* fmt, ...);
            void debug(const char* fmt, ...);
            void info(const char* fmt, ...);
            void warn(const char* fmt, ...);
            void error(const char* fmt, ...);
            void crit(const char* fmt, ...);

        private:
            size_t emitter_idx = 0;
            std::unique_ptr<LogEmitter> emitters[MAX_EMITTERS];
    };
}
