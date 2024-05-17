#include "mab_logger.h"

#define EXIT_LOG(level) if (lvl > level) {return; }
#define BPLATE_VPRINT(level) \
    std::va_list args; \
    va_start(args, fmt); \
    for (size_t i = 0; i < emitter_idx; i++) { \
        emitters[i]->emit(level, fmt, args); \
    } \
    va_end(args); \


mab::Logger::Logger() {
    addEmitter(std::make_unique<ConsoleEmitter>());
}


void mab::Logger::setLevel(LogLevel level) {
   lvl = level;
}

void mab::Logger::addEmitter(std::unique_ptr<LogEmitter> ptr) {
    if (emitter_idx >= MAX_EMITTERS) {
        throw std::overflow_error("Too many emitters added");
    }
    emitters[emitter_idx++] = std::move(ptr);
}

void mab::ConsoleEmitter::emit(LogLevel lvl, const char * fmt, std::va_list args) {
    printf("[%10s]: ", mab::LevelNames[lvl]);
    vprintf(fmt, args);
    std::cout << "\n";
}

void mab::Logger::trace(const char* fmt, ...) {
    EXIT_LOG(TRACE)
    BPLATE_VPRINT(TRACE)
}

void mab::Logger::debug(const char* fmt, ...) {
    EXIT_LOG(DEBUG)
    BPLATE_VPRINT(DEBUG)
}

void mab::Logger::info(const char* fmt, ...) {
    EXIT_LOG(INFO)
    BPLATE_VPRINT(INFO)
}

void mab::Logger::warn(const char* fmt, ...) {
    EXIT_LOG(WARN)
    BPLATE_VPRINT(WARN)
}

void mab::Logger::error(const char* fmt, ...) {
    EXIT_LOG(ERROR)
    BPLATE_VPRINT(ERROR)
}

void mab::Logger::crit(const char* fmt, ...) {
    EXIT_LOG(CRITICAL)
    BPLATE_VPRINT(CRITICAL)
}


