#include "mab_logger.h"

#define EXIT_LOG(level) if (lvl > level) {return; }
#define BPLATE_EMIT(level) \
    std::va_list args; \
    va_start(args, fmt); \
    for (size_t i = 0; i < emitter_idx; i++) { \
        emitters[i]->emit(level, fmt, args); \
    } \
    va_end(args); \

mab::LogLevel mab::Logger::lvl = DEBUG;

mab::Logger::Logger() {
    addEmitter(std::make_unique<ConsoleEmitter>());
}


void mab::Logger::setLevel(LogLevel level) {
   lvl = level;
}

const char * mab::Logger::getLevel() const {
   return mab::LEVEL_NAMES[lvl];
}

void mab::Logger::setLevel(const char * level) {
    for (size_t i = 0; i < mab::LEVEL_NAMES.size(); i++) {
        size_t j = 0;
        char x, y;
        bool eq = true;
        bool end = false;
        while (!end && eq) {
            x = mab::LEVEL_NAMES[i][j];
            y = (char)toupper(level[j]);
            ++j;
            end = (x == '\0') || (y == '\0');
            eq = (x == y);         
        }
        if (eq) {
            mab::Logger::setLevel((LogLevel)i);
            return;
        }
    }
}


void mab::Logger::addEmitter(std::unique_ptr<LogEmitter> ptr) {
    if (emitter_idx >= MAX_EMITTERS) {
        throw std::overflow_error("Too many emitters added");
    }
    emitters[emitter_idx++] = std::move(ptr);
}

void mab::ConsoleEmitter::emit(LogLevel lvl, const char * fmt, std::va_list args) const {
    printf("[%10s]: ", mab::LEVEL_NAMES[lvl]);
    vprintf(fmt, args);
    std::cout << "\n";
}

void mab::Logger::trace(const char* fmt, ...) const  {
    EXIT_LOG(TRACE)
    BPLATE_EMIT(TRACE)
}

void mab::Logger::debug(const char* fmt, ...) const  {
    EXIT_LOG(DEBUG)
    BPLATE_EMIT(DEBUG)
}

void mab::Logger::info(const char* fmt, ...) const  {
    EXIT_LOG(INFO)
    BPLATE_EMIT(INFO)
}

void mab::Logger::warn(const char* fmt, ...) const  {
    EXIT_LOG(WARN)
    BPLATE_EMIT(WARN)
}

void mab::Logger::error(const char* fmt, ...) const  {
    EXIT_LOG(ERROR)
    BPLATE_EMIT(ERROR)
}

void mab::Logger::crit(const char* fmt, ...) const  {
    EXIT_LOG(CRITICAL)
    BPLATE_EMIT(CRITICAL)
}


