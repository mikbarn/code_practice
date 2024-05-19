#ifndef MAB_LOGGER_H
#define MAB_LOGGER_H

#include <iostream>
#include <cstdarg>
#include <memory>
#include <stdexcept>
#include <array>

namespace mab {
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL
    };

    static const std::array<const char *, 6> LEVEL_NAMES = {"TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};

    class LogEmitter {
        public:
            virtual void emit(LogLevel lvl, const char *fmt, std::va_list args) const = 0;
    };

    class ConsoleEmitter: public LogEmitter{
        void emit(LogLevel lvl, const char * fmt, std::va_list args) const override;
    };

    class Logger {
        const static size_t MAX_EMITTERS = 8;
       
        public:
            static LogLevel lvl;
            Logger();
            Logger(LogLevel lvl);
            static void setLevel(LogLevel level);
            static void setLevel(const char * level);
            const char * getLevel() const;
            void addEmitter(std::unique_ptr<LogEmitter> ptr);

            void trace(const char* fmt, ...) const;
            void debug(const char* fmt, ...) const;
            void info(const char* fmt, ...) const;
            void warn(const char* fmt, ...) const;
            void error(const char* fmt, ...) const;
            void crit(const char* fmt, ...) const;

        private:
            size_t emitter_idx = 0;
            std::unique_ptr<LogEmitter> emitters[MAX_EMITTERS];
    };
   
}

#endif